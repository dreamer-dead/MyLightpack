/*
 * Settings.hpp
 *
 *  Created on: 29.07.2010
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

#pragma once

#include "DeviceTypesInfo.hpp"
#include "ConfigurationProfile.hpp"
#include "SettingsProfiles.hpp"
#include "SettingsReader.hpp"
#include "SettingsSignals.hpp"
#include "SettingsSource.hpp"

#include "common/DebugOut.hpp"
#include "common/defs.h"
#include "enums.hpp"
#include "types.h"

namespace SettingsScope {

/*!
  Provides access to persistent settings.
*/
class Settings : public SettingsSignals
               , private SettingsProfiles
               , public SettingsReader
{
public:
    class Overrides
    {
    public:
        Overrides() {}

        void setProfile(const QString& profileName);
        void setDebuglevel(Debug::DebugLevels level);
        void apply(ConfigurationProfile& profile) const;

    protected:
        struct OverridingValue
        {
            QVariant value;
            bool force;
        };

        void setValue(const QString & key, const QVariant& value, bool force = false)
        {
            const OverridingValue overridingValue = {value, force};
            m_overrides.insert(key, overridingValue);
        }

        typedef QMap<QString, OverridingValue> OverridesMap;

        OverridesMap m_overrides;
    };

    Settings();

    /*!
     * \brief Initialize reads settings file or create a new one and initializes it with default settings
     * \param applicationDirPath
     * \param isSetDebugLevelFromConfig
     * \return is settings file was present before initialization
     */
    static bool Initialize(const QString & applicationDirPath, const Overrides& overrides);
    static Settings * instance() { return m_instance.data(); }
    static void Shutdown();

    void resetDefaults();

    QStringList findAllProfiles() const;
    void loadOrCreateProfile(const QString & configName);
    void renameCurrentProfile(const QString & configName);
    void removeCurrentProfile();
    bool isProfileLoaded() const;

    QString getCurrentProfileName() const;
    /*!
      use with caution: if there is no profile loaded then it will throw access violation exception
     \see SettingsScope#Settings#getProfilesPath()
     \return QString path to current profile
    */
    QString getCurrentProfilePath() const;
    QString getProfilesPath() const;
    QString getApplicationDirPath() const;
    QString getMainConfigPath() const;

    // Main
    void setLanguage(const QString & language);
    void setDebugLevel(int debugLvl);
    void setIsApiEnabled(bool isEnabled);
    void setListenOnlyOnLoInterface(bool localOnly);
    void setApiPort(int apiPort);
    void setApiKey(const QString & apiKey);
    void setIsApiAuthEnabled(bool isEnabled);
    void setExpertModeEnabled(bool isEnabled);
    void setKeepLightsOnAfterExit(bool isEnabled);
    void setKeepLightsOnAfterLock(bool isEnabled);
    void setPingDeviceEverySecond(bool isEnabled);
    void setUpdateFirmwareMessageShown(bool isShown);
    void setConnectedDevice(SupportedDevices::DeviceType device);
    void setConnectedDeviceName(const QString & deviceName);
    void setHotkey(const QString &actionName, const QKeySequence &keySequence);
    void setAdalightSerialPortName(const QString & port);
    void setAdalightSerialPortBaudRate(const QString & baud);
    void setArdulightSerialPortName(const QString & port);
    void setArdulightSerialPortBaudRate(const QString & baud);
    // [Adalight | Ardulight | Lightpack | ... | Virtual]
    void setNumberOfLeds(SupportedDevices::DeviceType device, int numberOfLeds);
    void setColorSequence(SupportedDevices::DeviceType device, QString colorSequence);

    // Profile
    void setGrabSlowdown(int value);
    void setIsBacklightEnabled(bool isEnabled);
    void setGrabAvgColorsEnabled(bool isEnabled);
    void setSendDataOnlyIfColorsChanges(bool isEnabled);
    void setLuminosityThreshold(int value);
    void setMinimumLuminosityEnabled(bool value);

    // [Device]
    void setDeviceRefreshDelay(int value);
    void setDeviceBrightness(int value);
    void setDeviceSmooth(int value);
    void setDeviceColorDepth(int value);
    void setDeviceGamma(double gamma);

    void setGrabberType(Grab::GrabberType grabMode);

#ifdef D3D10_GRAB_SUPPORT
    void setDx1011GrabberEnabled(bool isEnabled);
#endif

    void setLightpackMode(Lightpack::Mode mode);
    void setMoodLampLiquidMode(bool isLiquidMode);
    void setMoodLampColor(QColor color);
    void setMoodLampSpeed(int value);

    void setLedCoefRed(int ledIndex, double value);
    void setLedCoefGreen(int ledIndex, double value);
    void setLedCoefBlue(int ledIndex, double value);

    void setLedSize(int ledIndex, QSize size);
    void setLedPosition(int ledIndex, QPoint position);
    void setLedEnabled(int ledIndex, bool isEnabled);

    void setLastReadUpdateId(const uint updateId);

private:
    void setValidLedCoef(int ledIndex, const QString & keyCoef, double coef);
    double getValidLedCoef(int ledIndex, const QString & keyCoef);

    void initDevicesMap();
    void migrateSettings();

public:
    QVariant pluginValue(const QString & pluginId, const QString & key) const;
    void setPluginValue(const QString & pluginId, const QString & key, const QVariant& value);

    class TestingOverrides : public Overrides {
    public:
        TestingOverrides() {}
        TestingOverrides& setConnectedDeviceForTests(SupportedDevices::DeviceType deviceType);
        TestingOverrides& setConfigVersionForTests(const BaseVersion& version);
    };

private:
    // This allows Settings to be deleted with private destructor.
    friend struct QScopedPointerDeleter<Settings>;

    Settings(const QString & mainConfigPath);
    ~Settings();

    void applyMainProfileOverrides(const Overrides& overrides);
    void applyCurrentProfileOverrides(const Overrides& overrides);
    void verifyMainProfile();
    void verifyCurrentProfile();

    QString m_applicationDirPath; // path to store app generated stuff
    DeviceTypesInfo m_deviceTypes; // device types, names and led numbers

    static QScopedPointer<Settings> m_instance;
};
} /*SettingsScope*/
