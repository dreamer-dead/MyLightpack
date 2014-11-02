/*
 * GrabManager.hpp
 *
 *  Created on: 26.07.2010
 *     Project: Lightpack
 *
 *  Copyright (c) 2010, 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  Lightpack a USB content-driving ambient lighting system
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

#pragma once

#include <QTimer>
#include <QList>

#include "GrabberContext.hpp"
#include "TimeEvaluations.hpp"
#include "LedWidgetsController.hpp"
#include "enums.hpp"

class GrabberBase;
class D3D10Grabber;

namespace SettingsScope {
class SettingsReader;
}

class GrabManager : public QObject {
    Q_OBJECT

public:
    GrabManager(const SettingsScope::SettingsReader *settings, QWidget *parent = 0);
    virtual ~GrabManager();

signals:
    void updateLedsColors(const QList<QRgb> & colors);
    void ambilightTimeOfUpdatingColors(double ms);
    void changeScreen();

public:

    // Grab options

    // Common options
    void setNumberOfLeds(int numberOfLeds);
    void reset();

public slots:
    void onGrabberTypeChanged(const Grab::GrabberType grabberType);
    void onGrabSlowdownChanged(int ms);
    void onGrabAvgColorsEnabledChanged(bool state);
    void onSendDataOnlyIfColorsEnabledChanged(bool state);
    void start(bool isGrabEnabled);
    void settingsProfileChanged(const QString &profileName);
    void setVisibleLedWidgets(bool state);
    void setColoredLedWidgets(bool state);
    void setWhiteLedWidgets(bool state);
    void onGrabberStateChangeRequested(bool isStartRequested);

private slots:
    void handleGrabbedColors();
    void timeoutUpdateFPS();
    void scaleLedWidgets(int screenIndexResized);
    void onFrameGrabAttempted(GrabResult result);
    void updateScreenGeometry();
    void onScreenCountChanged(int);

private:
    GrabberBase *queryGrabber(Grab::GrabberType grabber);
    void initGrabbers();
    GrabberBase *initGrabber(GrabberBase *grabber);
    void initColorLists(int numberOfLeds);
    void clearColorsNew();
    void clearColorsCurrent();
    void initLedWidgets(int numberOfLeds);

private:
    void initFromProfileSettings();

    QList<GrabberBase*> m_grabbers;
    GrabberBase *m_grabber;
    QList<QRect> m_lastScreenGeometry;

#ifdef D3D10_GRAB_SUPPORT
    D3D10Grabber *m_d3d10Grabber;
#endif

    QTimer m_timerGrab;
    QTimer m_timerUpdateFPS;
    LedWidgetsController m_widgetsController;
    QList<QRgb> m_grabResult;
    TimeEvaluations m_timeEval;

    QList<QRgb> m_colorsCurrent;
    QList<QRgb> m_colorsNew;

    bool m_isSendDataOnlyIfColorsChanged;
    bool m_avgColorsOnAllLeds;

    // Store last grabbing time in milliseconds
    double m_fpsMs;

    bool m_isGrabWidgetsVisible;
    GrabberContext m_grabberContext;
    const SettingsScope::SettingsReader * const m_settings;
};
