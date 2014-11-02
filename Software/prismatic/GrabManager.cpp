/*
 * GrabManager.cpp
 *
 *  Created on: 26.07.2010
 *      Author: Mike Shatohin (brunql)
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2010, 2011 Mike Shatohin, mikeshatohin [at] gmail.com
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

#include <QtCore/qmath.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

#include "GrabberBase.hpp"
#include "SettingsReader.hpp"
#include "WinAPIGrabber.hpp"
#include "WinAPIGrabberEachWidget.hpp"
#include "QtGrabber.hpp"
#include "QtGrabberEachWidget.hpp"
#include "X11Grabber.hpp"
#include "MacOSGrabber.hpp"
#include "D3D9Grabber.hpp"
#include "D3D10Grabber.hpp"
#include "GrabManager.hpp"
#include "common/DebugOut.hpp"
#include "ui/GrabWidget.hpp"

using namespace SettingsScope;

#ifdef D3D10_GRAB_SUPPORT

#include "LightpackApplication.hpp"

static void *GetMainWindowHandle()
{
    return getLightpackApp()->getMainWindowHandle();
}
#endif

GrabManager::GrabManager(const SettingsScope::SettingsReader *settings,
                         QWidget *parent)
    : QObject(parent)
    , m_timerGrab(this)
    , m_timerUpdateFPS(this)
    , m_widgetsController(parent)
    , m_fpsMs(0)
    , m_isGrabWidgetsVisible(false)
    , m_settings(settings) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    qRegisterMetaType<GrabResult>("GrabResult");

    Q_ASSERT(m_settings);

    m_isSendDataOnlyIfColorsChanged = m_settings->isSendDataOnlyIfColorsChanges();

    initGrabbers();
    m_grabber = queryGrabber(m_settings->getGrabberType());

    connect(&m_timerUpdateFPS, &QTimer::timeout,
            this, &GrabManager::timeoutUpdateFPS);
    m_timerUpdateFPS.setSingleShot(false);
    m_timerUpdateFPS.start(500);

    initColorLists(MaximumNumberOfLeds::Default);
    m_widgetsController.initLedWidgets(MaximumNumberOfLeds::Default);

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(scaleLedWidgets(int)));
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(onScreenCountChanged(int)));

    updateScreenGeometry();

    initFromProfileSettings();

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "initialized";
}

GrabManager::~GrabManager() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_grabber = NULL;

    for (int i = 0; i < m_grabbers.size(); i++) {
        if (m_grabbers[i]) {
            DEBUG_OUT << "deleting " << m_grabbers[i]->name();
            delete m_grabbers[i];
            m_grabbers[i] = NULL;
        }
    }

    m_grabbers.clear();

#ifdef D3D10_GRAB_SUPPORT
    delete m_d3d10Grabber;
    m_d3d10Grabber = NULL;
#endif
}

void GrabManager::start(bool isGrabEnabled) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << isGrabEnabled;

    clearColorsNew();

    if (!m_grabber)
        return;

    if (isGrabEnabled) {
        m_timerUpdateFPS.start();
        m_grabber->startGrabbing();
    } else {
        clearColorsCurrent();
        m_timerUpdateFPS.stop();
        m_grabber->stopGrabbing();
        emit ambilightTimeOfUpdatingColors(0);
    }
}

void GrabManager::onGrabberTypeChanged(const Grab::GrabberType grabberType) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << grabberType;

    bool isStartNeeded = false;
    if (m_grabber != NULL) {
        isStartNeeded = m_grabber->isGrabbingStarted();
#ifdef D3D10_GRAB_SUPPORT
        isStartNeeded = isStartNeeded || (m_d3d10Grabber != NULL && m_d3d10Grabber->isGrabbingStarted());
#endif
        m_grabber->stopGrabbing();
    }

    m_grabber = queryGrabber(grabberType);

    if (isStartNeeded) {
#ifdef D3D10_GRAB_SUPPORT
        if (m_settings->isDx1011GrabberEnabled())
            m_d3d10Grabber->startGrabbing();
        else
            m_grabber->startGrabbing();
#else
        m_grabber->startGrabbing();
#endif
    }
}

void GrabManager::onGrabberStateChangeRequested(bool isStartRequested) {
#ifdef D3D10_GRAB_SUPPORT
    D3D10Grabber *grabber = static_cast<D3D10Grabber *>(sender());
    if (grabber != m_grabber) {
        if (isStartRequested) {
            if (m_settings->isDx1011GrabberEnabled()) {
                m_grabber->stopGrabbing();
                grabber->startGrabbing();
            }
        } else {
            m_grabber->startGrabbing();
            grabber->stopGrabbing();
        }
    } else {
        qCritical() << Q_FUNC_INFO << " there is no grabber to take control by some reason";
    }
#else
    Q_UNUSED(isStartRequested)
#endif
}

void GrabManager::onGrabSlowdownChanged(int ms) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << ms;
    if (m_grabber)
        m_grabber->setGrabInterval(ms);
    else
        qWarning() << Q_FUNC_INFO << "trying to change grab slowdown while there is no grabber";
}

void GrabManager::onGrabAvgColorsEnabledChanged(bool state) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;
    m_avgColorsOnAllLeds = state;
}

void GrabManager::onSendDataOnlyIfColorsEnabledChanged(bool state) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;
    m_isSendDataOnlyIfColorsChanged = state;
}

void GrabManager::setNumberOfLeds(int numberOfLeds) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

    initColorLists(numberOfLeds);
    m_widgetsController.toggleLedsVisibility(numberOfLeds, m_isGrabWidgetsVisible);
}

void GrabManager::reset() {
    clearColorsCurrent();
}

void GrabManager::settingsProfileChanged(const QString& profileName) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    Q_UNUSED(profileName)

    initFromProfileSettings();
}

void GrabManager::setVisibleLedWidgets(bool state) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;

    m_isGrabWidgetsVisible = state;
    m_widgetsController.showWidgets(m_isGrabWidgetsVisible);
}

void GrabManager::setColoredLedWidgets(bool state) {
    // This slot is directly connected to radioButton toggled(bool) signal
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    if (state)
        m_widgetsController.fillLedsBackgroundColored();
}

void GrabManager::setWhiteLedWidgets(bool state)
{
    // This slot is directly connected to radioButton toggled(bool) signal
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    if (state)
        m_widgetsController.fillLedsBackgroundWhite();
}

void GrabManager::handleGrabbedColors() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    if (m_grabber == NULL) {
        qCritical() << Q_FUNC_INFO << "m_grabber == NULL";
        return;
    }

    // Temporary switch off updating colors
    // if one of LED widgets resizing or moving
    if (m_widgetsController.inResizingOrMoving()) {
        m_timerGrab.start(50); // check in 50 ms
        return;
    }    

    bool isColorsChanged = false;
    int avgR = 0, avgG = 0, avgB = 0;
    int countGrabEnabled = 0;

    const int colorsListSize = m_widgetsController.widgetsCount();
    if (m_colorsNew.size() != colorsListSize) {
        m_colorsNew.clear();
        for (int i = 0; i < colorsListSize; i++) {
            m_colorsNew << 0;
        }
    }

    if (m_avgColorsOnAllLeds) {
        for (int i = 0; i < colorsListSize; i++) {
            if (m_widgetsController.widget(i).isAreaEnabled()) {
                    avgR += qRed(m_colorsNew[i]);
                    avgG += qGreen(m_colorsNew[i]);
                    avgB += qBlue(m_colorsNew[i]);
                    countGrabEnabled++;
            }
        }

        if (countGrabEnabled != 0) {
            avgR /= countGrabEnabled;
            avgG /= countGrabEnabled;
            avgB /= countGrabEnabled;
        }

        // Set one AVG color to all LEDs
        const QRgb avgColor = qRgb(avgR, avgG, avgB);
        for (int ledIndex = 0; ledIndex < colorsListSize; ++ledIndex) {
            if (m_widgetsController.widget(ledIndex).isAreaEnabled()) {
                m_colorsNew[ledIndex] = avgColor;
            }
        }
    }

//    // White balance
//    for (int i = 0; i < m_ledWidgets.size(); i++)
//    {
//        QRgb rgb = m_colorsNew[i];

//        unsigned r = qRed(rgb)   * m_ledWidgets[i]->getCoefRed();
//        unsigned g = qGreen(rgb) * m_ledWidgets[i]->getCoefGreen();
//        unsigned b = qBlue(rgb)  * m_ledWidgets[i]->getCoefBlue();

//        if (r > 0xff) r = 0xff;
//        if (g > 0xff) g = 0xff;
//        if (b > 0xff) b = 0xff;

//        m_colorsNew[i] = qRgb(r, g, b);
//    }

    for (int i = 0; i < colorsListSize; i++)
    {
        if (m_colorsCurrent[i] != m_colorsNew[i])
        {
            m_colorsCurrent[i] = m_colorsNew[i];
            isColorsChanged = true;
        }
    }

    if ((m_isSendDataOnlyIfColorsChanged == false) || isColorsChanged)
    {
        emit updateLedsColors(m_colorsCurrent);
    }

    m_fpsMs = m_timeEval.howLongItEnd();
    m_timeEval.howLongItStart();

}

void GrabManager::timeoutUpdateFPS()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO;
    emit ambilightTimeOfUpdatingColors(m_fpsMs);
}

void GrabManager::updateScreenGeometry()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    m_lastScreenGeometry.clear();
    for (int i = 0; i < QApplication::desktop()->screenCount(); ++i) {
        m_lastScreenGeometry.append(QApplication::desktop()->screenGeometry(i));
    }
    emit changeScreen();
    if (m_grabber == NULL)
    {
        qCritical() << Q_FUNC_INFO << "m_grabber == NULL";
        return;
    }

}

void GrabManager::onScreenCountChanged(int)
{
    updateScreenGeometry();
}

void GrabManager::scaleLedWidgets(int screenIndexResized) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "screenIndexResized:" << screenIndexResized;

    Q_ASSERT(screenIndexResized <= m_lastScreenGeometry.size());
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(screenIndexResized);
    const QRect lastScreenGeometry = m_lastScreenGeometry[screenIndexResized];

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "screen " << screenIndexResized << " is resized to " << screenGeometry;
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "was " << lastScreenGeometry;

    m_widgetsController.scaleLedWidgets(screenGeometry, lastScreenGeometry);
    m_lastScreenGeometry[screenIndexResized] = screenGeometry;
}

void GrabManager::initGrabbers()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    // We need an address of the QList to write into.
    m_grabberContext.grabWidgets = &m_widgetsController.grabbedAreas();
    m_grabberContext.grabResult = &m_colorsNew;

    for (int i = 0; i < Grab::GrabbersCount; i++)
        m_grabbers.append(NULL);

#ifdef WINAPI_GRAB_SUPPORT
    m_grabbers[Grab::GrabberTypeWinAPI] = initGrabber(new WinAPIGrabber(NULL, &m_grabberContext));
#endif

#ifdef D3D9_GRAB_SUPPORT
    m_grabbers[Grab::GrabberTypeD3D9] = initGrabber(new D3D9Grabber(NULL, &m_grabberContext));
#endif

#ifdef X11_GRAB_SUPPORT
    m_grabbers[Grab::GrabberTypeX11] = initGrabber(new X11Grabber(NULL, &m_grabberContext));
#endif

#ifdef MAC_OS_CG_GRAB_SUPPORT
    m_grabbers[Grab::GrabberTypeMacCoreGraphics] = initGrabber(new MacOSGrabber(NULL, &m_grabberContext));
#endif
#ifdef QT_GRAB_SUPPORT
    //TODO: migrate Qt grabbers to the new hierarchy
    m_grabbers[Grab::GrabberTypeQtEachWidget] = initGrabber(new QtGrabberEachWidget(NULL, &m_grabberContext));
    m_grabbers[Grab::GrabberTypeQt] = initGrabber(new QtGrabber(NULL, &m_grabberContext));
#endif
#ifdef WINAPI_EACH_GRAB_SUPPORT
    m_grabbers[Grab::GrabberTypeWinAPIEachWidget] = initGrabber(new WinAPIGrabberEachWidget(NULL, &m_grabberContext));
#endif
#ifdef D3D10_GRAB_SUPPORT
    m_d3d10Grabber = static_cast<D3D10Grabber *>(initGrabber(new D3D10Grabber(NULL, &m_grabberContext, &GetMainWindowHandle)));
    connect(m_d3d10Grabber, SIGNAL(grabberStateChangeRequested(bool)), SLOT(onGrabberStateChangeRequested(bool)));
    connect(getLightpackApp(), SIGNAL(postInitialization()), m_d3d10Grabber,  SLOT(init()));
#endif
}

GrabberBase *GrabManager::initGrabber(GrabberBase * grabber) {
    QMetaObject::invokeMethod(grabber, "setGrabInterval", Qt::QueuedConnection, Q_ARG(int, m_settings->getGrabSlowdown()));
    bool isConnected = connect(grabber, SIGNAL(frameGrabAttempted(GrabResult)), this, SLOT(onFrameGrabAttempted(GrabResult)), Qt::QueuedConnection);
    Q_ASSERT_X(isConnected, "connecting grabber to grabManager", "failed");
    Q_UNUSED(isConnected);

    return grabber;
}

GrabberBase *GrabManager::queryGrabber(Grab::GrabberType grabberType) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "grabberType:" << grabberType;
    GrabberBase *result;

    if (m_grabbers[grabberType] != NULL) {
        result = m_grabbers[grabberType];
    } else {
        qCritical() << Q_FUNC_INFO << "unsupported for the platform grabber type: " << grabberType << ", using QtGrabber";
        result = m_grabbers[Grab::GrabberTypeQt];
    }

    result->setGrabInterval(m_settings->getGrabSlowdown());

    return result;
}

void GrabManager::onFrameGrabAttempted(GrabResult grabResult) {
    if (grabResult == GrabResultOk) {
        handleGrabbedColors();
    }
}

void GrabManager::initColorLists(int numberOfLeds) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

    m_colorsCurrent.clear();
    m_colorsNew.clear();

    for (int i = 0; i < numberOfLeds; i++) {
        m_colorsCurrent << 0;
        m_colorsNew     << 0;
    }
}

void GrabManager::clearColorsNew() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    for (int i = 0; i < m_colorsNew.size(); i++) {
        m_colorsNew[i] = 0;
    }
}

void GrabManager::clearColorsCurrent() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    for (int i = 0; i < m_colorsCurrent.size(); i++) {
        m_colorsCurrent[i] = 0;
    }
}

void GrabManager::initFromProfileSettings() {
    m_isSendDataOnlyIfColorsChanged = m_settings->isSendDataOnlyIfColorsChanges();
    m_avgColorsOnAllLeds = m_settings->isGrabAvgColorsEnabled();

    setNumberOfLeds(m_settings->getNumberOfConnectedDeviceLeds());
}
