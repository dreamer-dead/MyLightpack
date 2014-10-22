/*
 * LedDeviceManager.cpp
 *
 *  Created on: 17.04.2011
 *      Author: Timur Sattarov && Mike Shatohin
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

#include <qglobal.h>
#include <type_traits>

#include "LedDeviceManager.hpp"
#include "devices/LedDeviceLightpack.hpp"

#ifdef Q_OS_WIN
#include "devices/LedDeviceAlienFx.hpp"
#endif

#include "devices/LedDeviceAdalight.hpp"
#include "devices/LedDeviceArdulight.hpp"
#include "devices/LedDeviceVirtual.hpp"
#include "Settings.hpp"
#include "third_party/qtutils/include/QTUtils.hpp"

using namespace SettingsScope;

namespace {
struct CommandContext {
    QList<QRgb> savedColors;
    bool isColorsSaved;
    int savedRefreshDelay;
    int savedColorDepth;
    int savedSmoothSlowdown;
    double savedGamma;
    int savedBrightness;
    int savedLuminosityThreshold;
    bool savedIsMinimumLuminosityEnabled;
    QString savedColorSequence;
};

template <void (LedDeviceManager::*signal)()>
struct VoidCommandRunner {
    static void run(LedDeviceManager& manager, const CommandContext&) {
        emit (manager.*signal)();
    }

    static void saveContext(CommandContext&) {}
};

template <void (LedDeviceManager::*signal)()>
struct VoidCommandRunnerNoEmit {
    static void run(LedDeviceManager& manager, const CommandContext&) {
        (manager.*signal)();
    }

    static void saveContext(CommandContext&) {}
};

template <typename T>
struct ParamBaseType {
    typedef typename std::remove_const<
        typename std::remove_reference<T>::type
    >::type Type;
};

template <typename TParam,
          void (LedDeviceManager::*signal)(TParam),
          typename ParamBaseType<TParam>::Type CommandContext::* contextMember>
struct BaseCommandRunner {
    typedef TParam ParamType;

    static void run(LedDeviceManager& manager, const CommandContext& context) {
        emit (manager.*signal)(context.*contextMember);
    }

    static void saveContext(CommandContext& context, TParam value) {
        context.*contextMember = value;
    }
};

// Base class for custom command SetColors.
typedef BaseCommandRunner<
    const QList<QRgb>&,
    &LedDeviceManager::ledDeviceSetColors,
    &CommandContext::savedColors>
SetColorsCommandBase;

// Implementation for custom command SetColors.
struct SetColorsCommandRunner : private SetColorsCommandBase {
    typedef SetColorsCommandBase::ParamType ParamType;

    static void run(LedDeviceManager& manager, const CommandContext& context) {
        Q_ASSERT(context.isColorsSaved);
        SetColorsCommandBase::run(manager, context);
    }

    static void saveColors(CommandContext& context, const ParamType& colors) {
        SetColorsCommandBase::saveContext(context, colors);
    }

    // No real work.
    static void saveContext(CommandContext&, const ParamType&) {
    }
};
}

class LedDeviceManager::CommandDispatcher {
public:
    typedef void (*CommandRunner)(LedDeviceManager&, const CommandContext&);

    CommandDispatcher(LedDeviceManager& owner)
            : m_isLastCommandCompleted(true)
            , m_owner(owner) {
        m_cmdTimeoutTimer.setInterval(100);
        connect(&m_cmdTimeoutTimer, &QTimer::timeout,
                &owner, &LedDeviceManager::ledDeviceCommandTimedOut);
        m_context.isColorsSaved = false;
    }

    template <typename Command, typename ...ValueTypes>
    void postCommand(ValueTypes... values) {
        DEBUG_MID_LEVEL << Q_FUNC_INFO
                        << "Is last command completed:" << m_isLastCommandCompleted;

        if (m_isLastCommandCompleted) {
            m_cmdTimeoutTimer.start();
            m_isLastCommandCompleted = false;
            Command::run(m_owner, m_context);
        } else {
            Command::saveContext(m_context, values...);
            appendCommand(&Command::run);
        }
    }

    void appendCommand(CommandRunner cmdRunner) {
        DEBUG_MID_LEVEL << Q_FUNC_INFO << cmdRunner;
        Q_ASSERT(cmdRunner);

        if (!m_cmdQueue.contains(cmdRunner))
            m_cmdQueue.append(cmdRunner);
    }

    void commandCompleted(bool ok) {
        DEBUG_MID_LEVEL << Q_FUNC_INFO << ok;

        m_cmdTimeoutTimer.stop();

        if (ok) {
            processNextCommand();
        } else {
            m_cmdQueue.clear();
            m_isLastCommandCompleted = true;
        }
    }

    void resetState() {
        commandCompleted(false);
    }

    bool isColorsSaved() const { return m_context.isColorsSaved; }

    const QList<QRgb>& savedColors() const { return m_context.savedColors; }

    void setSavedColors(const QList<QRgb>& colors) {
        m_context.savedColors = colors;
        m_context.isColorsSaved = true;
    }

private:
    void processNextCommand() {
        DEBUG_MID_LEVEL << Q_FUNC_INFO << m_cmdQueue;

        if (!m_cmdQueue.isEmpty()) {
            const CommandRunner runner = m_cmdQueue.takeFirst();
            Q_ASSERT(runner);

            DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "processing cmd = " << runner;
            m_cmdTimeoutTimer.start();
            runner(m_owner, m_context);
        }
    }

    bool m_isLastCommandCompleted;
    QList<CommandRunner> m_cmdQueue;
    QTimer m_cmdTimeoutTimer;
    CommandContext m_context;
    LedDeviceManager& m_owner;
};

LedDeviceManager::LedDeviceManager(const SettingsScope::SettingsReader* settings,
                                   QObject *parent)
    : QObject(parent)
    , m_backlightStatus(Backlight::StatusOn)
    , m_ledDevice(CURRENT_LOCATION)
    , m_settings(settings) {
    Q_ASSERT(settings);
    for (int i = 0; i < SupportedDevices::DeviceTypesCount; ++i)
        m_ledDevices.append(NULL);
}

LedDeviceManager::~LedDeviceManager()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    for (int i = 0; i < m_ledDevices.size(); i++) {
        if(m_ledDevices[i])
            m_ledDevices[i]->close();

        if (m_ledDevices[i] != m_ledDevice.get())
            delete m_ledDevices[i];
    }
    const bool joined = m_ledDevice.join(1000);
    Q_ASSERT(joined);
    Q_UNUSED(joined);
}

void LedDeviceManager::init()
{
    m_commandDispatcher.reset(new CommandDispatcher(*this));

    initLedDevice();
}

void LedDeviceManager::recreateLedDevice(const SupportedDevices::DeviceType deviceType)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    Q_UNUSED(deviceType)

    disconnectCurrentLedDevice();

    initLedDevice();
}

void LedDeviceManager::switchOnLeds()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    m_backlightStatus = Backlight::StatusOn;
    Q_ASSERT(m_commandDispatcher.data());
    if (m_commandDispatcher->isColorsSaved())
        emit ledDeviceSetColors(m_commandDispatcher->savedColors());
}

void LedDeviceManager::setColors(const QList<QRgb>& colors)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << " m_backlightStatus = " << m_backlightStatus;

    if (m_backlightStatus != Backlight::StatusOn)
        return;

    Q_ASSERT(m_commandDispatcher.data());
    // Always save the colors array.
    m_commandDispatcher->setSavedColors(colors);
    m_commandDispatcher->postCommand<SetColorsCommandRunner>(colors);
}

void LedDeviceManager::switchOffLeds()
{
    typedef VoidCommandRunnerNoEmit<&LedDeviceManager::processOffLeds> SwitchOffLeds;
    m_commandDispatcher->postCommand<SwitchOffLeds>();
}

void LedDeviceManager::processOffLeds()
{
    m_backlightStatus = Backlight::StatusOff;

    emit ledDeviceOffLeds();
}

void LedDeviceManager::setRefreshDelay(int value)
{
    typedef BaseCommandRunner<
        int,
        &LedDeviceManager::ledDeviceSetRefreshDelay,
        &CommandContext::savedRefreshDelay> SetRefreshDelay;

    m_commandDispatcher->postCommand<SetRefreshDelay>(value);
}

void LedDeviceManager::setColorDepth(int value)
{
    typedef BaseCommandRunner<
        int,
        &LedDeviceManager::ledDeviceSetColorDepth,
        &CommandContext::savedColorDepth> SetColorDepth;
    m_commandDispatcher->postCommand<SetColorDepth>(value);
}

void LedDeviceManager::setSmoothSlowdown(int value)
{
    typedef BaseCommandRunner<
        int,
        &LedDeviceManager::ledDeviceSetSmoothSlowdown,
        &CommandContext::savedSmoothSlowdown> SetSmoothSlowdown;
    m_commandDispatcher->postCommand<SetSmoothSlowdown>(value);
}

void LedDeviceManager::setGamma(double value)
{
    typedef BaseCommandRunner<
        double,
        &LedDeviceManager::ledDeviceSetGamma,
        &CommandContext::savedGamma> SetGamma;
    m_commandDispatcher->postCommand<SetGamma>(value);
}

void LedDeviceManager::setBrightness(int value)
{
    typedef BaseCommandRunner<
        int,
        &LedDeviceManager::ledDeviceSetBrightness,
        &CommandContext::savedBrightness> SetBrightness;
    m_commandDispatcher->postCommand<SetBrightness>(value);
}

void LedDeviceManager::setLuminosityThreshold(int value)
{
    typedef BaseCommandRunner<
        int,
        &LedDeviceManager::ledDeviceSetLuminosityThreshold,
        &CommandContext::savedLuminosityThreshold> SetLuminosityThreshold;
    m_commandDispatcher->postCommand<SetLuminosityThreshold>(value);
}

void LedDeviceManager::setMinimumLuminosityEnabled(bool value)
{
    typedef BaseCommandRunner<
        bool,
        &LedDeviceManager::ledDeviceSetMinimumLuminosityEnabled,
        &CommandContext::savedIsMinimumLuminosityEnabled>
    SetMinimumLuminosityEnabled;
    m_commandDispatcher->postCommand<SetMinimumLuminosityEnabled>(value);
}

void LedDeviceManager::setColorSequence(QString value)
{
    typedef BaseCommandRunner<
        QString,
        &LedDeviceManager::ledDeviceSetColorSequence,
        &CommandContext::savedColorSequence> SetColorSequence;
    m_commandDispatcher->postCommand<SetColorSequence>(value);
}

void LedDeviceManager::requestFirmwareVersion()
{
    typedef VoidCommandRunner<
        &LedDeviceManager::ledDeviceRequestFirmwareVersion> RequestFirmwareVersion;
    m_commandDispatcher->postCommand<RequestFirmwareVersion>();
}

void LedDeviceManager::updateDeviceSettings()
{
    typedef VoidCommandRunner<
        &LedDeviceManager::ledDeviceUpdateDeviceSettings> UpdateDeviceSettings;
    m_commandDispatcher->postCommand<UpdateDeviceSettings>();
}

void LedDeviceManager::updateWBAdjustments()
{
    typedef VoidCommandRunner<
        &LedDeviceManager::ledDeviceUpdateWBAdjustments> UpdateWBAdjustments;
    m_commandDispatcher->postCommand<UpdateWBAdjustments>();
}

void LedDeviceManager::ledDeviceCommandCompleted(bool ok)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << ok;

    m_commandDispatcher->commandCompleted(ok);
    emit ioDeviceSuccess(ok);
}

void LedDeviceManager::initLedDevice()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    Q_ASSERT(m_commandDispatcher.data());
    m_commandDispatcher->resetState();

    const SupportedDevices::DeviceType connectedDevice = m_settings->getConnectedDevice();
    if (m_ledDevices[connectedDevice] == NULL) {
        m_ledDevices[connectedDevice] = createLedDevice(connectedDevice);
        connectLedDevice(m_ledDevices[connectedDevice]);
    } else {
        Q_ASSERT(m_ledDevices[connectedDevice] != m_ledDevice.get());
        disconnectCurrentLedDevice();
        connectLedDevice(m_ledDevices[connectedDevice]);
    }
    emit ledDeviceUpdateDeviceSettings();
    emit ledDeviceOpen();
}

AbstractLedDevice * LedDeviceManager::createLedDevice(SupportedDevices::DeviceType deviceType)
{
    using namespace SupportedDevices;

#if !defined(Q_OS_WIN)
    if (deviceType == DeviceTypeAlienFx) {
        qWarning() << Q_FUNC_INFO << "AlienFx not supported on current platform";

        Settings::instance()->setConnectedDevice(DefaultDeviceType);
        deviceType = Settings::instance()->getConnectedDevice();
    }
#endif /* Q_OS_WIN */

    switch (deviceType) {
    case DeviceTypeLightpack:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::LightpackDevice";
        return new LedDeviceLightpack();

    case DeviceTypeAlienFx:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::AlienFxDevice";

#       ifdef Q_OS_WIN
        return new LedDeviceAlienFx();
#       else
        break;
#       endif /* Q_OS_WIN */

    case DeviceTypeAdalight:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::AdalightDevice";
        return new LedDeviceAdalight(
            m_settings->getAdalightSerialPortName(),
            m_settings->getAdalightSerialPortBaudRate());

    case DeviceTypeArdulight:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::ArdulightDevice";
        return new LedDeviceArdulight(
            m_settings->getArdulightSerialPortName(),
            m_settings->getArdulightSerialPortBaudRate());

    case DeviceTypeVirtual:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::VirtualDevice";
        return new LedDeviceVirtual(m_settings->getDeviceGamma(),
                                    m_settings->getDeviceBrightness());

    default:
        break;
    }

    qFatal("%s %s%d%s", Q_FUNC_INFO,
           "Create LedDevice fail. deviceType = '", deviceType,
           "'. Application exit.");

    return NULL; // Avoid compiler warning
}

void LedDeviceManager::connectLedDevice(AbstractLedDevice * device) {
    if (device == NULL) {
        qWarning() << Q_FUNC_INFO << "device == NULL";
        return;
    }

    QtUtils::makeQueuedConnector(device, this)
        .connect(SIGNAL(commandCompleted(bool)), SLOT(ledDeviceCommandCompleted(bool)))
        .connect(SIGNAL(firmwareVersion(QString)), SLOT(firmwareVersion(QString)))
        .connect(SIGNAL(ioDeviceSuccess(bool)), SLOT(ioDeviceSuccess(bool)))
        .connect(SIGNAL(openDeviceSuccess(bool)), SLOT(openDeviceSuccess(bool)))
        .connect(SIGNAL(colorsUpdated(QList<QRgb>)),
                 SLOT(setColors_VirtualDeviceCallback(QList<QRgb>)));

    QtUtils::makeQueuedConnector(this, device)
        .connect(SIGNAL(ledDeviceOpen()), SLOT(open()))
        .connect(SIGNAL(ledDeviceSetColors(QList<QRgb>)), SLOT(setColors(QList<QRgb>)))
        .connect(SIGNAL(ledDeviceOffLeds()), SLOT(switchOffLeds()))
        .connect(SIGNAL(ledDeviceSetRefreshDelay(int)), SLOT(setRefreshDelay(int)))
        .connect(SIGNAL(ledDeviceSetColorDepth(int)), SLOT(setColorDepth(int)))
        .connect(SIGNAL(ledDeviceSetSmoothSlowdown(int)), SLOT(setSmoothSlowdown(int)))
        .connect(SIGNAL(ledDeviceSetGamma(double)), SLOT(setGamma(double)))
        .connect(SIGNAL(ledDeviceSetBrightness(int)), SLOT(setBrightness(int)))
        .connect(SIGNAL(ledDeviceSetColorSequence(QString)), SLOT(setColorSequence(QString)))
        .connect(SIGNAL(ledDeviceSetLuminosityThreshold(int)) ,SLOT(setLuminosityThreshold(int)))
        .connect(SIGNAL(ledDeviceSetMinimumLuminosityEnabled(bool)),
                 SLOT(setMinimumLuminosityThresholdEnabled(bool)))
        .connect(SIGNAL(ledDeviceRequestFirmwareVersion()), SLOT(requestFirmwareVersion()))
        .connect(SIGNAL(ledDeviceUpdateWBAdjustments()), SLOT(updateDeviceSettings()))
        .connect(SIGNAL(ledDeviceUpdateDeviceSettings()), SLOT(updateDeviceSettings()));

    m_ledDevice.init(device);
}

void LedDeviceManager::disconnectCurrentLedDevice()
{
    const AbstractLedDevice* const device = m_ledDevice.get();
    if (device == NULL) {
        qWarning() << Q_FUNC_INFO << "device == NULL";
        return;
    }

    QtUtils::makeConnector(device, this)
        .disconnect(SIGNAL(commandCompleted(bool)), SLOT(ledDeviceCommandCompleted(bool)))
        .disconnect(SIGNAL(firmwareVersion(QString)), SLOT(firmwareVersion(QString)))
        .disconnect(SIGNAL(ioDeviceSuccess(bool)), SLOT(ioDeviceSuccess(bool)))
        .disconnect(SIGNAL(openDeviceSuccess(bool)), SLOT(openDeviceSuccess(bool)))
        .disconnect(SIGNAL(colorsUpdated(QList<QRgb>)),
                    SLOT(setColors_VirtualDeviceCallback(QList<QRgb>)));

    QtUtils::makeConnector(this, device)
        .disconnect(SIGNAL(ledDeviceOpen()), SLOT(open()))
        .disconnect(SIGNAL(ledDeviceSetColors(QList<QRgb>)), SLOT(setColors(QList<QRgb>)))
        .disconnect(SIGNAL(ledDeviceOffLeds()), SLOT(switchOffLeds()))
        .disconnect(SIGNAL(ledDeviceSetRefreshDelay(int)), SLOT(setRefreshDelay(int)))
        .disconnect(SIGNAL(ledDeviceSetColorDepth(int)), SLOT(setColorDepth(int)))
        .disconnect(SIGNAL(ledDeviceSetSmoothSlowdown(int)), SLOT(setSmoothSlowdown(int)))
        .disconnect(SIGNAL(ledDeviceSetGamma(double)), SLOT(setGamma(double)))
        .disconnect(SIGNAL(ledDeviceSetBrightness(int)), SLOT(setBrightness(int)))
        .disconnect(SIGNAL(ledDeviceSetColorSequence(QString)), SLOT(setColorSequence(QString)))
        .disconnect(SIGNAL(ledDeviceSetLuminosityThreshold(int)), SLOT(setLuminosityThreshold(int)))
        .disconnect(SIGNAL(ledDeviceSetMinimumLuminosityEnabled(bool)),
                    SLOT(setMinimumLuminosityThresholdEnabled(bool)))
        .disconnect(SIGNAL(ledDeviceRequestFirmwareVersion()), SLOT(requestFirmwareVersion()))
        .disconnect(SIGNAL(ledDeviceUpdateWBAdjustments()), SLOT(updateDeviceSettings()))
        .disconnect(SIGNAL(ledDeviceUpdateDeviceSettings()), SLOT(updateDeviceSettings()));
}

void LedDeviceManager::ledDeviceCommandTimedOut()
{
    ledDeviceCommandCompleted(false);
    emit ioDeviceSuccess(false);
}
