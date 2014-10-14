/*
 * LightpackApplication.hpp
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

#pragma once

#include <QScopedPointer>

#include "EndSessionDetector.hpp"
#include "LedDeviceManager.hpp"
#include "qtsingleapplication.h"

#define getLightpackApp() static_cast<LightpackApplication *>(QCoreApplication::instance())

class ApiServer;
class GrabManager;
class LightpackPluginInterface;
class MoodLampManager;
class PluginsManager;
class SettingsWindow;

namespace SettingsScope {
class Settings;
}

class LightpackApplication : public QtSingleApplication
{
    Q_OBJECT
public:
    LightpackApplication(int &argc, char **argv);
    ~LightpackApplication();

    void initializeAll(const QString & appDirPath);
#ifdef Q_OS_WIN
    bool winEventFilter ( MSG * msg, long * result );
    HWND getMainWindowHandle();
#endif
    SettingsWindow * settingsWnd() const { return m_settingsWindow.data(); }
    const SettingsScope::SettingsReader * settingsReader() const;
    SettingsScope::Settings * settings() const;

    enum ErrorCodes {
        OK_ErrorCode                            = 0,
        WrongCommandLineArgument_ErrorCode      = 1,
        AppDirectoryCreationFail_ErrorCode      = 2,
        OpenLogsFail_ErrorCode                  = 3,
        QFatalMessageHandler_ErrorCode          = 4,
        LogsDirecroryCreationFail_ErrorCode     = 5,
        // Append new ErrorCodes here
        JustEpicFail_ErrorCode                  = 93
    };

signals:
    void clearColorBuffers();
    void postInitialization(); /*!< emits at the end of initializeAll method*/

public slots:
    void setStatusChanged(Backlight::Status);
    void setBacklightChanged(Lightpack::Mode);
    void free();

private slots:
    void requestBacklightStatus();
    void setDeviceLockViaAPI(DeviceLocked::DeviceLockStatus status, QList<QString> modules);
    void profileSwitch(const QString & configName);
    void settingsChanged();
    void showLedWidgets(bool visible);
    void setColoredLedWidget(bool colored);
    void onFocusChanged(QWidget *, QWidget *);
    void quitFromWizard(int result);
    void processMessageNoGUI(const QString&);

private:
    void processCommandLineArguments();
    void outputMessage(QString message) const;
    void printVersionsSoftwareQtOS() const;
    bool checkSystemTrayAvailability() const;
    void startApiServer();
    void startLedDeviceManager();
    void initGrabManager();
    void startPluginManager();
    void startBacklight();

    void runWizardLoop(bool isInitFromSettings);

    virtual void commitData(QSessionManager &sessionManager);

private:
    // Helper getters for using in connect/disconnect
    const GrabManager* grabManager() const { return m_grabManager.data(); }
    const MoodLampManager* moodLampManager() const {
        return m_moodlampManager.data();
    }

    const PluginsManager* pluginsManager() const {
        return m_pluginManager.data();
    }

    const LightpackPluginInterface* lightpackPlugin() const {
        return m_pluginInterface.data();
    }

    QMutex m_mutex;
    QScopedPointer<SettingsWindow> m_settingsWindow;
    QScopedPointer<ApiServer> m_apiServer;
    QScopedPointer<LedDeviceManager> m_ledDeviceManager;
    // These objects will be deleted by their deleteLater slots.
    QThread *m_LedDeviceManagerThread;
    QThread *m_apiServerThread;
    QScopedPointer<GrabManager> m_grabManager;
    QScopedPointer<MoodLampManager> m_moodlampManager;
    QScopedPointer<PluginsManager> m_pluginManager;
    QScopedPointer<LightpackPluginInterface> m_pluginInterface;

    QString m_applicationDirPath;
    bool m_isDebugLevelObtainedFromCmdArgs;
    bool m_noGui;
    DeviceLocked::DeviceLockStatus m_deviceLockStatus;
    bool m_isSettingsWindowActive;
    Backlight::Status m_backlightStatus;

    typedef std::vector<QSharedPointer<QAbstractNativeEventFilter> > EventFilters;
    EventFilters m_EventFilters;
};
