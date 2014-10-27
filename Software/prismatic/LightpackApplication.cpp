/*
 * LightpackApplication.cpp
 *
 *  Created on: 06.09.2011
 *      Author: Mike Shatohin (brunql)
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <stdio.h>
#include <iostream>
#include <memory>

#include "ApiServer.hpp"
#include "LightpackApplication.hpp"
#include "LightpackPluginInterface.hpp"
#include "PluginsManager.hpp"
#include "Plugin.hpp"
#include "LightpackCommandLineParser.hpp"
#include "Settings.hpp"
#include "SettingsDefaults.hpp"
#include "devices/LedDeviceLightpack.hpp"
#include "version.h"
#include "ui/SettingsWindow.hpp"
#include "wizard/Wizard.hpp"

using namespace std;
using namespace SettingsScope;
using namespace QtUtils;

namespace {
void printVersionsSoftwareQtOS() {
    if (g_debugLevel > 0) {
#       ifdef GIT_REVISION
        qDebug() << "Prismatik:" << VERSION_STR << "rev." << GIT_REVISION;
#       else
        qDebug() << "Prismatik:" << VERSION_STR;
#       endif

        qDebug() << "Build with Qt verison:" << QT_VERSION_STR;
        qDebug() << "Qt version currently in use:" << qVersion();

#       ifdef Q_OS_WIN
        switch(QSysInfo::windowsVersion()){
        case QSysInfo::WV_NT:       qDebug() << "Windows NT (operating system version 4.0)"; break;
        case QSysInfo::WV_2000:     qDebug() << "Windows 2000 (operating system version 5.0)"; break;
        case QSysInfo::WV_XP:       qDebug() << "Windows XP (operating system version 5.1)"; break;
        case QSysInfo::WV_2003:     qDebug() << "Windows Server 2003, Windows Server 2003 R2, Windows Home Server, Windows XP Professional x64 Edition (operating system version 5.2)"; break;
        case QSysInfo::WV_VISTA:    qDebug() << "Windows Vista, Windows Server 2008 (operating system version 6.0)"; break;
        case QSysInfo::WV_WINDOWS7: qDebug() << "Windows 7, Windows Server 2008 R2 (operating system version 6.1)"; break;
        default:                    qDebug() << "Unknown windows version:" << QSysInfo::windowsVersion();
        }
#       elif defined(Q_OS_LINUX)
        // TODO: print some details about OS (cat /proc/partitions? lsb_release -a?)
#       elif defined(Q_OS_MAC)
        qDebug() << "Mac OS";
#       else
        qDebug() << "Unknown operation system";
#       endif
    }
}

void outputMessage(const QString& message) {
#ifdef Q_OS_WIN
    QMessageBox::information(NULL, "Prismatik", message, QMessageBox::Ok);
#else
    fprintf(stderr, "%s\n", message.toStdString().c_str());
#endif
}

bool checkSystemTrayAvailability()
{
#   ifdef Q_OS_LINUX
    // When you add lightpack in the Startup in Ubuntu (10.04), tray starts later than the application runs.
    // Check availability tray every second for 20 seconds.
    for (int i = 0; i < 20; i++)
    {
        if (QSystemTrayIcon::isSystemTrayAvailable())
            break;

        QThread::sleep(1);
    }
#   endif

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(0, "Prismatik", "I couldn't detect any system tray on this system.");
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Systray couldn't be detected, running in trayless mode";
        return false;
    }
    return true;
}

}

LightpackApplication::ExitedThreadsGuard::~ExitedThreadsGuard() {
    Q_ASSERT(registeredThreadsCount() == 0);
}

LightpackApplication::LightpackApplication(int &argc, char **argv)
    : QtSingleApplication(argc, argv)
    , m_apiServer(CURRENT_LOCATION)
    , m_ledDeviceManager(CURRENT_LOCATION) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
}

LightpackApplication::~LightpackApplication() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
}

Settings * LightpackApplication::settings() const {
    return Settings::instance();
}

const SettingsReader * LightpackApplication::settingsReader() const {
    return SettingsReader::instance();
}

void LightpackApplication::processMessageNoGUI(const QString& message) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << message;
    if ("on" == message)
        setStatusChanged(Backlight::StatusOn);
    else if ("off" == message)
        setStatusChanged(Backlight::StatusOff);
    else if (message.startsWith("set-profile ")) {
        const QString profile = message.mid(12);
        DEBUG_LOW_LEVEL << "Query to change profile: " << profile;
        settings()->loadOrCreateProfile(profile);
    }
}

void LightpackApplication::initializeAll(const QString & appDirPath)
{
    setApplicationName("Prismatik");
    setOrganizationName("Woodenshark LLC");
    setApplicationVersion(VERSION_STR);
    setQuitOnLastWindowClosed(false);

    m_applicationDirPath = appDirPath;
    m_noGui = false;

    processCommandLineArguments();

    printVersionsSoftwareQtOS();
    if (isRunning())
        return;

    SettingsScope::Settings::Overrides overrides;
    if (m_isDebugLevelObtainedFromCmdArgs)
        overrides.setDebuglevel(static_cast<Debug::DebugLevels>(g_debugLevel));
    if (!Settings::Initialize(m_applicationDirPath, overrides) && !m_noGui) {
        runWizardLoop(false);
    }

    m_isSettingsWindowActive = false;

    if (!m_noGui)
    {
        bool trayAvailable = checkSystemTrayAvailability();

        m_settingsWindow.reset(new SettingsWindow());
        if (trayAvailable) {
            m_settingsWindow->setVisible(false); /* Load to tray */
            m_settingsWindow->createTrayIcon();
        }
        else
            m_settingsWindow->setVisible(true);
        m_settingsWindow->connectSignalsSlots();
        connect(this, SIGNAL(postInitialization()), m_settingsWindow.data(), SLOT(onPostInit()));
        // Process messages from another instances in SettingsWindow
        connect(this, SIGNAL(messageReceived(QString)), m_settingsWindow.data(), SLOT(processMessage(QString)));
        connect(this, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(onFocusChanged(QWidget*,QWidget*)));
    } else {
        // Process messages for profile switching, etc
        connect(this, SIGNAL(messageReceived(QString)), this, SLOT(processMessageNoGUI(QString)));
    }
    // Register QMetaType for Qt::QueuedConnection
    qRegisterMetaType< QList<QRgb> >("QList<QRgb>");
    qRegisterMetaType< QList<QString> >("QList<QString>");
    qRegisterMetaType<Lightpack::Mode>("Lightpack::Mode");
    qRegisterMetaType<Backlight::Status>("Backlight::Status");
    qRegisterMetaType<DeviceLocked::DeviceLockStatus>("DeviceLocked::DeviceLockStatus");
    qRegisterMetaType< QList<Plugin*> >("QList<Plugin*>");

    if (settings()->isBacklightEnabled())
    {
        m_backlightStatus = Backlight::StatusOn;
    } else {
        m_backlightStatus = Backlight::StatusOff;
    }
    m_deviceLockStatus = DeviceLocked::Unlocked;

    startLedDeviceManager();

    startApiServer();

    initGrabManager();

    if (!m_noGui)
    {
        connect(m_settingsWindow.data(), SIGNAL(backlightStatusChanged(Backlight::Status)), this, SLOT(setStatusChanged(Backlight::Status)));
        m_settingsWindow->startBacklight();
    }

    this->settingsChanged();

    startPluginManager();

    // This is a place to register some more EventFilters.
    m_eventFilters.push_back(QSharedPointer<QAbstractNativeEventFilter>(new EndSessionDetector()));
    for (EventFilters::const_iterator it = m_eventFilters.begin(); it != m_eventFilters.end(); ++it)
        this->installNativeEventFilter(it->data());

    QtUtils::Connector<> connector(this);
    connector.connect(SIGNAL(aboutToQuit()), SLOT(free()));
    emit postInitialization();
}

void LightpackApplication::runWizardLoop(bool isInitFromSettings)
{
    Wizard wizard(isInitFromSettings);
    connect(&wizard, SIGNAL(finished(int)), this, SLOT(quitFromWizard(int)));
    wizard.setWindowFlags(Qt::WindowStaysOnTopHint);
    wizard.show();
    this->exec();
}

#ifdef Q_OS_WIN
HWND LightpackApplication::getMainWindowHandle() {
    // to get HWND sometimes needed to activate window
//    winFocus(m_settingsWindow, true);
    return reinterpret_cast<HWND>(m_settingsWindow->winId());
}

bool LightpackApplication::winEventFilter ( MSG * msg, long * result ) {
    Q_UNUSED(result);

    const unsigned char POWER_RESUME  = 0x01;
    const unsigned char POWER_SUSPEND = 0x02;
    static unsigned char processed = 0x00;
    if ( WM_POWERBROADCAST == msg->message ) {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "WM_POWERBROADCAST "  <<  msg->wParam << " " << processed;
        switch(msg->wParam) {
        case PBT_APMRESUMEAUTOMATIC :
            if ( !(POWER_RESUME & processed) ) {
                ledDeviceManager()->switchOnLeds();
                processed = POWER_RESUME;
                return true;
            }
            break;
        case PBT_APMSUSPEND :
            if ( !(POWER_SUSPEND & processed) ) {
                ledDeviceManager()->switchOffLeds();
                processed = POWER_SUSPEND;
                return true;
            }
            break;
        }
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "hwnd = " << msg->hwnd;
    }
    return false;
}
#endif

void LightpackApplication::setStatusChanged(Backlight::Status status)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << status;
    m_backlightStatus = status;
    startBacklight();
}

void LightpackApplication::setBacklightChanged(Lightpack::Mode mode)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << mode;
    settings()->setLightpackMode(mode);
    if (!m_noGui)
        m_settingsWindow->setModeChanged(mode);
    startBacklight();
}

void LightpackApplication::setDeviceLockViaAPI(DeviceLocked::DeviceLockStatus status, QList<QString> modules)
{
    Q_UNUSED(modules);
    m_deviceLockStatus = status;

    if (m_grabManager == NULL)
        qFatal("%s m_grabManager == NULL", Q_FUNC_INFO);
    startBacklight();
}

void LightpackApplication::startBacklight()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "m_backlightStatus =" << m_backlightStatus
                    << "m_deviceLockStatus =" << m_deviceLockStatus;

    QtUtils::makeConnector(settings(), moodLampManager())
        .connect(SIGNAL(moodLampColorChanged(QColor)), SLOT(setCurrentColor(QColor)))
        .connect(SIGNAL(moodLampSpeedChanged(int)), SLOT(setLiquidModeSpeed(int)))
        .connect(SIGNAL(moodLampLiquidModeChanged(bool)), SLOT(setLiquidMode(bool)));

    bool isBacklightEnabled = (m_backlightStatus == Backlight::StatusOn || m_backlightStatus == Backlight::StatusDeviceError);
    bool isCanStart = (isBacklightEnabled && m_deviceLockStatus == DeviceLocked::Unlocked);

    m_pluginInterface->resultBacklightStatus(m_backlightStatus);

    settings()->setIsBacklightEnabled(isBacklightEnabled);

    const Lightpack::Mode lightpackMode = settings()->getLightpackMode();
    switch (lightpackMode)
    {
    case Lightpack::AmbilightMode:
        m_grabManager->start(isCanStart);
        m_moodlampManager->start(false);
        break;

    case Lightpack::MoodLampMode:
        m_grabManager->start(false);
        m_moodlampManager->start(isCanStart);
        break;

    default:
        qWarning() << Q_FUNC_INFO << "lightpackMode unsupported value =" << lightpackMode;
        break;
    }

    if (m_backlightStatus == Backlight::StatusOff)
        ledDeviceManager()->switchOffLeds();
    else
        ledDeviceManager()->switchOnLeds();


    switch (m_backlightStatus)
    {
    case Backlight::StatusOff:
        emit clearColorBuffers();
        break;

    default:
        qWarning() << Q_FUNC_INFO << "status contains crap =" << m_backlightStatus;
        break;
    }
}
void LightpackApplication::onFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(now);
    Q_UNUSED(old);
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << (this->activeWindow() != NULL ? this->activeWindow()->metaObject()->className() : "null");
    if(!m_isSettingsWindowActive && (this->activeWindow() == m_settingsWindow.data())) {
        m_settingsWindow->onFocus();
        m_isSettingsWindowActive = true;
    } else {
        if(m_isSettingsWindowActive && (this->activeWindow() == NULL)) {
            m_settingsWindow->onBlur();
            m_isSettingsWindowActive = false;
        }
    }
}

void LightpackApplication::quitFromWizard(int result)
{
    Q_UNUSED(result);
    quit();
}

void LightpackApplication::processCommandLineArguments()
{
    g_debugLevel = SettingsScope::Main::DebugLevelDefault;
    m_isDebugLevelObtainedFromCmdArgs = false;

    LightpackCommandLineParser parser;
    if (!parser.parse(arguments())) {
        outputMessage(parser.errorText());
        ::exit(WrongCommandLineArgument_ErrorCode);
    }

    if (parser.isSetHelp()) {
        outputMessage(parser.helpText());
        ::exit(0);
    }
    if (parser.isSetVersion())
    {
        const QString versionString =
            "Version: " VERSION_STR "\n"
#ifdef GIT_REVISION
            "Revision: " GIT_REVISION "\n"
#endif
            "Build with Qt version " QT_VERSION_STR "\n";
        outputMessage(versionString);
        ::exit(0);
    }

    // these two options are mutually exclusive
    if (parser.isSetNoGUI()) {
        m_noGui = true;
        DEBUG_LOW_LEVEL <<  "Application running no_GUI mode";
    } else if (parser.isSetWizard()) {
        SettingsScope::Settings::Overrides overrides;
        overrides.setDebuglevel(static_cast<Debug::DebugLevels>(g_debugLevel));
        bool isInitFromSettings = Settings::Initialize(m_applicationDirPath, overrides);
        runWizardLoop(isInitFromSettings);
    }

    // 'on' and 'off' are mutually exclusive also
    if (parser.isSetBacklightOff()) {
        if (!isRunning()) {
            LedDeviceLightpack lightpackDevice;
            lightpackDevice.switchOffLeds();
        } else {
            sendMessage("off");
        }
        ::exit(0);
    }
    else if (parser.isSetBacklightOn())
    {
        if (isRunning())
            sendMessage("on");
        ::exit(0);
    }

    if (parser.isSetProfile()) {
        if (isRunning())
            sendMessage("set-profile " + parser.profileName());
        ::exit(0);
    }

    if (parser.isSetDebuglevel())
    {
        g_debugLevel = parser.debugLevel();
        m_isDebugLevelObtainedFromCmdArgs = true;
    }

    if (m_isDebugLevelObtainedFromCmdArgs)
        qDebug() << "Debug level" << g_debugLevel;
}

void LightpackApplication::startApiServer()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Start API server";

    QScopedPointer<ApiServer> apiServer(new ApiServer());
    apiServer.data()->setInterface(m_pluginInterface.data());

    connect(this, SIGNAL(clearColorBuffers()),
            apiServer.data(), SIGNAL(clearColorBuffers()));

    makeConnector(settings(), apiServer.data())
        .connect(SIGNAL(apiServerSettingsChanged()), SLOT(apiServerSettingsChanged()))
        .connect(SIGNAL(apiKeyChanged(const QString &)), SLOT(updateApiKey(const QString &)))
        .connect(SIGNAL(lightpackNumberOfLedsChanged(int)), SIGNAL(updateApiDeviceNumberOfLeds(int)))
        .connect(SIGNAL(adalightNumberOfLedsChanged(int)), SIGNAL(updateApiDeviceNumberOfLeds(int)))
        .connect(SIGNAL(ardulightNumberOfLedsChanged(int)), SIGNAL(updateApiDeviceNumberOfLeds(int)))
        .connect(SIGNAL(virtualNumberOfLedsChanged(int)), SIGNAL(updateApiDeviceNumberOfLeds(int)));

    if (!m_noGui)
    {
        connect(apiServer.data(), SIGNAL(errorOnStartListening(QString)),
                m_settingsWindow.data(), SLOT(onApiServer_ErrorOnStartListening(QString)));
    }

    Q_ASSERT(ledDeviceManager());
    connect(ledDeviceManager(), SIGNAL(setColors_VirtualDeviceCallback(QList<QRgb>)),
            lightpackPlugin(), SLOT(updateColors(QList<QRgb>)), Qt::QueuedConnection);

    apiServer.data()->firstStart();
    m_apiServer.init(apiServer.take());
}

void LightpackApplication::startLedDeviceManager()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    QScopedPointer<LedDeviceManager> ledManager(new LedDeviceManager(settingsReader()));
    m_pluginInterface.reset(new LightpackPluginInterface(NULL));

    // LightpackPluginInterface connections.
    const auto& lightpackPlugin2this = makeConnector(lightpackPlugin(), this);
    lightpackPlugin2this.connect(
        SIGNAL(updateDeviceLockStatus(DeviceLocked::DeviceLockStatus, QList<QString>)),
        SLOT(setDeviceLockViaAPI(DeviceLocked::DeviceLockStatus, QList<QString>)));

    makeQueuedConnector(m_pluginInterface, ledManager)
        .connect(SIGNAL(updateLedsColors(const QList<QRgb> &)), SLOT(setColors(QList<QRgb>)))
        .connect(SIGNAL(updateGamma(double)), SLOT(setGamma(double)))
        .connect(SIGNAL(updateBrightness(int)), SLOT(setBrightness(int)))
        .connect(SIGNAL(updateSmooth(int)), SLOT(setSmoothSlowdown(int)));

    lightpackPlugin2this.connect(SIGNAL(requestBacklightStatus()),
                                 SLOT(requestBacklightStatus()));

    // LedDeviceManager connections.
    makeQueuedConnector(settings(), ledManager.data())
        .connect(SIGNAL(deviceColorDepthChanged(int)), SLOT(setColorDepth(int)))
        .connect(SIGNAL(deviceSmoothChanged(int)), SLOT(setSmoothSlowdown(int)))
        .connect(SIGNAL(deviceRefreshDelayChanged(int)), SLOT(setRefreshDelay(int)))
        .connect(SIGNAL(deviceGammaChanged(double)), SLOT(setGamma(double)))
        .connect(SIGNAL(deviceBrightnessChanged(int)), SLOT(setBrightness(int)))
        .connect(SIGNAL(luminosityThresholdChanged(int)), SLOT(setLuminosityThreshold(int)))
        .connect(SIGNAL(minimumLuminosityEnabledChanged(bool)),
                 SLOT(setMinimumLuminosityEnabled(bool)))
        .connect(SIGNAL(ledCoefBlueChanged(int,double)), SLOT(updateWBAdjustments()))
        .connect(SIGNAL(ledCoefRedChanged(int,double)), SLOT(updateWBAdjustments()))
        .connect(SIGNAL(ledCoefGreenChanged(int,double)), SLOT(updateWBAdjustments()));

    if (m_noGui) {
        lightpackPlugin2this.connect(SIGNAL(updateProfile(QString)),
                                     SLOT(profileSwitch(QString)));
        lightpackPlugin2this.connect(SIGNAL(updateStatus(Backlight::Status)),
                                     SLOT(setStatusChanged(Backlight::Status)));
    } else {
        // LightpackPluginInterface-Window connections.
        makeConnector(m_pluginInterface, m_settingsWindow)
            .connect(SIGNAL(updateLedsColors(const QList<QRgb> &)),
                     SLOT(updateVirtualLedsColors(QList<QRgb>)))
            .connect(SIGNAL(updateDeviceLockStatus(DeviceLocked::DeviceLockStatus, QList<QString>)),
                     SLOT(setDeviceLockViaAPI(DeviceLocked::DeviceLockStatus, QList<QString>)))
            .connect(SIGNAL(updateProfile(QString)), SLOT(profileSwitch(QString)))
            .connect(SIGNAL(updateStatus(Backlight::Status)), SLOT(setBacklightStatus(Backlight::Status)));

        // Window-LedManager connections.
        makeQueuedConnector(m_settingsWindow, ledManager)
            .connect(SIGNAL(requestFirmwareVersion()), SLOT(requestFirmwareVersion()))
            .connect(SIGNAL(switchOffLeds()), SLOT(switchOffLeds()))
            .connect(SIGNAL(switchOnLeds()), SLOT(switchOnLeds()));
        makeQueuedConnector(ledManager, m_settingsWindow)
            .connect(SIGNAL(openDeviceSuccess(bool)), SLOT(ledDeviceOpenSuccess(bool)))
            .connect(SIGNAL(ioDeviceSuccess(bool)), SLOT(ledDeviceCallSuccess(bool)))
            .connect(SIGNAL(firmwareVersion(QString)),
                     SLOT(ledDeviceFirmwareVersionResult(QString)))
            .connect(SIGNAL(setColors_VirtualDeviceCallback(QList<QRgb>)),
                     SLOT(updateVirtualLedsColors(QList<QRgb>)));
    }
    m_ledDeviceManager.init(ledManager.take());
    QMetaObject::invokeMethod(m_ledDeviceManager.get(), "init", Qt::QueuedConnection);

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "end";
}

void LightpackApplication::startPluginManager() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    const QString pluginsDir = QtUtils::pathCombine(
        settings()->getApplicationDirPath(),
        PluginsManager::defaultPluginsDir());
    m_pluginManager.reset(new PluginsManager(pluginsDir, NULL));

    connect(this, SIGNAL(destroyed()), pluginsManager(), SLOT(StopPlugins()));

    if (!m_noGui)
    {
        connect(m_settingsWindow.data(), SIGNAL(reloadPlugins()),
                pluginsManager(), SLOT(reloadPlugins()));
        connect(pluginsManager(), SIGNAL(updatePlugin(QList<Plugin*>)),
                m_settingsWindow.data(), SLOT(updatePlugin(QList<Plugin*>)), Qt::QueuedConnection);
        connect(pluginsManager(), SIGNAL(updatePlugin(QList<Plugin*>)),
                lightpackPlugin(), SLOT(updatePlugin(QList<Plugin*>)), Qt::QueuedConnection);
    }

    m_pluginManager->reloadPlugins();
}

void LightpackApplication::initGrabManager()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    m_grabManager.reset(new GrabManager(settingsReader()));
    m_moodlampManager.reset(new MoodLampManager(NULL));

    m_moodlampManager->initFromSettings();

    connect(settings(), SIGNAL(grabberTypeChanged(const Grab::GrabberType &)),
            grabManager(), SLOT(onGrabberTypeChanged(const Grab::GrabberType &)), Qt::QueuedConnection);
    connect(settings(), SIGNAL(grabSlowdownChanged(int)),
            grabManager(), SLOT(onGrabSlowdownChanged(int)), Qt::QueuedConnection);
    connect(settings(), SIGNAL(grabAvgColorsEnabledChanged(bool)),
            grabManager(), SLOT(onGrabAvgColorsEnabledChanged(bool)), Qt::QueuedConnection);

    connect(settings(), SIGNAL(profileLoaded(const QString &)),
            grabManager(), SLOT(settingsProfileChanged(const QString &)), Qt::QueuedConnection);
    connect(settings(), SIGNAL(currentProfileInited(const QString &)),
            grabManager(), SLOT(settingsProfileChanged(const QString &)), Qt::QueuedConnection);
    // Connections to signals which will be connected to ILedDevice
    if (!m_noGui)
    {
        connect(m_settingsWindow.data(), SIGNAL(showLedWidgets(bool)),
                this, SLOT(showLedWidgets(bool)));
        connect(m_settingsWindow.data(), SIGNAL(setColoredLedWidget(bool)),
                this, SLOT(setColoredLedWidget(bool)));

        // GrabManager to this
        connect(grabManager(), SIGNAL(ambilightTimeOfUpdatingColors(double)),
                m_settingsWindow.data(), SLOT(refreshAmbilightEvaluated(double)));
    }

    connect(grabManager(), SIGNAL(updateLedsColors(const QList<QRgb> &)),
            ledDeviceManager(), SLOT(setColors(QList<QRgb>)), Qt::QueuedConnection);
    connect(moodLampManager(), SIGNAL(updateLedsColors(const QList<QRgb> &)),
            ledDeviceManager(), SLOT(setColors(QList<QRgb>)), Qt::QueuedConnection);
    connect(grabManager(), SIGNAL(updateLedsColors(const QList<QRgb> &)),
            lightpackPlugin(), SLOT(updateColors(const QList<QRgb> &)), Qt::QueuedConnection);
    connect(moodLampManager(), SIGNAL(updateLedsColors(const QList<QRgb> &)),
            lightpackPlugin(), SLOT(updateColors(const QList<QRgb> &)), Qt::QueuedConnection);
    connect(grabManager(), SIGNAL(ambilightTimeOfUpdatingColors(double)),
            lightpackPlugin(), SLOT(refreshAmbilightEvaluated(double)));
    connect(grabManager(), SIGNAL(changeScreen(QRect)),
            lightpackPlugin(), SLOT(refreshScreenRect(QRect)));
}

void LightpackApplication::commitData(QSessionManager &sessionManager)
{
    Q_UNUSED(sessionManager);

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Off leds before quit";

    if (ledDeviceManager())
    {
        // Disable signals with new colors
        disconnect(m_settingsWindow.data(), SIGNAL(updateLedsColors(QList<QRgb>)),
                   ledDeviceManager(), SLOT(setColors(QList<QRgb>)));
        disconnect(apiServer(), SIGNAL(updateLedsColors(QList<QRgb>)),
                   ledDeviceManager(), SLOT(setColors(QList<QRgb>)));

        // Process all currently pending signals
        QApplication::processEvents(QEventLoop::AllEvents, 1000);

        // Send signal and process it
        ledDeviceManager()->switchOffLeds();
        QApplication::processEvents(QEventLoop::AllEvents, 1000);
    }
}

void LightpackApplication::profileSwitch(const QString & configName)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << configName;
    settings()->loadOrCreateProfile(configName);
}

void LightpackApplication::settingsChanged()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_pluginInterface->changeProfile(settings()->getCurrentProfileName());

    m_grabManager->onSendDataOnlyIfColorsEnabledChanged(settings()->isSendDataOnlyIfColorsChanges());

    m_moodlampManager->setSendDataOnlyIfColorsChanged(settings()->isSendDataOnlyIfColorsChanges());
    m_moodlampManager->setCurrentColor(settings()->getMoodLampColor());
    m_moodlampManager->setLiquidModeSpeed(settings()->getMoodLampSpeed());
    m_moodlampManager->setLiquidMode(settings()->isMoodLampLiquidMode());

    const bool canStartManager = (settings()->isBacklightEnabled() &&
                                  m_deviceLockStatus == DeviceLocked::Unlocked);
    switch (settings()->getLightpackMode())
    {
    case Lightpack::AmbilightMode:
        m_grabManager->start(canStartManager);
        m_moodlampManager->start(false);
        break;

    default:
        m_grabManager->start(false);
        m_moodlampManager->start(canStartManager);
        break;
    }

    //Force update colors on device for start ping device
    m_grabManager->reset();
    m_moodlampManager->reset();
}

void LightpackApplication::showLedWidgets(bool visible)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << visible;
    m_grabManager->setVisibleLedWidgets(visible);
}

void LightpackApplication::setColoredLedWidget(bool colored)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << colored;
    m_grabManager->setColoredLedWidgets(colored);
    m_grabManager->setWhiteLedWidgets(!colored);
}

void LightpackApplication::requestBacklightStatus()
{
    m_pluginInterface->resultBacklightStatus(m_backlightStatus);
}

void LightpackApplication::free() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    disconnect(this, SIGNAL(focusChanged(QWidget*,QWidget*)),
               this, SLOT(onFocusChanged(QWidget*,QWidget*)));
    if (m_moodlampManager)
        m_moodlampManager->start(false);

    if (m_grabManager)
        m_grabManager->start(false);

    if (m_pluginManager)
        m_pluginManager->stopPlugins();

    QApplication::processEvents(QEventLoop::AllEvents, 1000);

    if (apiServer()) {
        emit m_apiServer->finished();
        const bool threadJoined = m_apiServer.join(1000);
        Q_ASSERT(threadJoined);
        Q_UNUSED(threadJoined);
    }

    if (ledDeviceManager()) {
        emit m_ledDeviceManager->finished();
        const bool threadJoined = m_ledDeviceManager.join(1000);
        Q_ASSERT(threadJoined);
        Q_UNUSED(threadJoined);
    }
    QApplication::processEvents(QEventLoop::AllEvents, 1000);
}
