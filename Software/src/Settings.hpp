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

#include <QColor>
#include <QVariant>
#include <QMap>
#include <QMutex>
#include <QScopedPointer>
#include <QSettings>

#include "BaseVersion.hpp"
#include "SettingsDefaults.hpp"
#include "enums.hpp"
#include "../common/defs.h"
#include "debug.h"
#include "types.h"

class BaseVersion;
class SettingsTest;

namespace SettingsScope
{

class SettingsSource
{
public:
    virtual QVariant value(const QString & key) const = 0;
    virtual void setValue(const QString & key, const QVariant & value) = 0;
    virtual bool contains(const QString& key) const = 0;
    virtual void remove(const QString& key) = 0;
    virtual void sync() {}
    virtual ~SettingsSource() {}
};

class ConfigurationProfile
{
public:
    typedef SettingsSource* (*SettingsSourceFabricFunc)(const QString&);

    static void setSourceFabric(SettingsSourceFabricFunc fabric);

    ConfigurationProfile();

    bool init(const QString& path, const QString& name);
    bool isInitialized() const { return m_settings; }
    const QString& name() const { return m_name; }
    const QString& path() const { return m_path; }

    QVariant value(const QString & key) const;
    QVariant valueOrDefault(const QString & key, const QVariant& defaultValue) const;
    bool contains(const QString& key) const;
    void setValue(const QString & key, const QVariant & value, bool force = true);
    void remove(const QString& key);

    bool beginBatchUpdate();
    void endBatchUpdate();
    void reset();

    struct ScopedBatchUpdateGuard
    {
        ConfigurationProfile& m_profile;

        ScopedBatchUpdateGuard(ConfigurationProfile& profile)
            : m_profile(profile)
        {
            m_profile.beginBatchUpdate();
        }

        ~ScopedBatchUpdateGuard() { m_profile.endBatchUpdate(); }
    };

private:
    mutable QMutex m_mutex;
    bool m_isInBatchUpdate;
    QString m_name;
    QString m_path;
    QScopedPointer<SettingsSource> m_settings;
};

class SettingsSignals : public QObject {
    Q_OBJECT

protected:
    SettingsSignals() {}
    ~SettingsSignals() {}

signals:
    void profileLoaded(const QString &);
    void currentProfileNameChanged(const QString &);
    void currentProfileRemoved();
    void currentProfileInited(const QString &);
    void apiServerSettingsChanged();
    void apiKeyChanged(const QString &);
    void expertModeEnabledChanged(bool);
    void keepLightsOnAfterExitChanged(bool isEnabled);
    void keepLightsOnAfterLockChanged(bool isEnabled);
    void pingDeviceEverySecondEnabledChanged(bool);

    void languageChanged(const QString &);
    void debugLevelChanged(int);
    void updateFirmwareMessageShownChanged(bool isShown);
    void connectedDeviceChanged(const SupportedDevices::DeviceType device);
    void hotkeyChanged(const QString &actionName, const QKeySequence & newKeySequence, const QKeySequence &oldKeySequence);
    void adalightSerialPortNameChanged(const QString & port);
    void adalightSerialPortBaudRateChanged(const QString & baud);
    void ardulightSerialPortNameChanged(const QString & port);
    void ardulightSerialPortBaudRateChanged(const QString & baud);
    void lightpackNumberOfLedsChanged(int numberOfLeds);
    void adalightNumberOfLedsChanged(int numberOfLeds);
    void ardulightNumberOfLedsChanged(int numberOfLeds);
    void virtualNumberOfLedsChanged(int numberOfLeds);
    void grabSlowdownChanged(int value);
    void backlightEnabledChanged(bool isEnabled);
    void grabAvgColorsEnabledChanged(bool isEnabled);
    void sendDataOnlyIfColorsChangesChanged(bool isEnabled);
    void luminosityThresholdChanged(int value);
    void minimumLuminosityEnabledChanged(bool value);
    void deviceRefreshDelayChanged(int value);
    void deviceBrightnessChanged(int value);
    void deviceSmoothChanged(int value);
    void deviceColorDepthChanged(int value);
    void deviceGammaChanged(double gamma);
    void deviceColorSequenceChanged(QString value);
    void grabberTypeChanged(const Grab::GrabberType grabMode);
    void dx1011GrabberEnabledChanged(const bool isEnabled);
    void lightpackModeChanged(const Lightpack::Mode mode);
    void moodLampLiquidModeChanged(bool isLiquidMode);
    void moodLampColorChanged(const QColor color);
    void moodLampSpeedChanged(int value);
    void ledCoefRedChanged(int ledIndex, double value);
    void ledCoefGreenChanged(int ledIndex, double value);
    void ledCoefBlueChanged(int ledIndex, double value);
    void ledSizeChanged(int ledIndex, const QSize &size);
    void ledPositionChanged(int ledIndex, const QPoint &position);
    void ledEnabledChanged(int ledIndex, bool isEnabled);
};

class SettingsProfiles {
public:
    QVariant valueMain(const QString & key) const;
    QVariant value(const QString & key) const;

protected:
    // forwarding to m_mainConfig object
    void setValueMain(const QString & key, const QVariant & value);
    // forwarding to m_currentProfile object
    void setValue(const QString & key, const QVariant & value);

    ConfigurationProfile m_mainProfile;
    ConfigurationProfile m_currentProfile;
};

class SettingsReader {
public:
    SettingsReader(SettingsProfiles& profiles)
        : m_profiles(profiles)
    {}

    // Main
    QString getLanguage() const;
    int getDebugLevel() const;
    bool isApiEnabled() const;
    bool isListenOnlyOnLoInterface() const;
    int getApiPort() const;
    QString getApiAuthKey() const;
    bool isApiAuthEnabled() const;
    bool isExpertModeEnabled() const;
    bool isKeepLightsOnAfterExit() const;
    bool isKeepLightsOnAfterLock() const;
    bool isPingDeviceEverySecond() const;
    bool isUpdateFirmwareMessageShown() const;
    SupportedDevices::DeviceType getConnectedDevice() const;
    QString getConnectedDeviceName() const;
    QKeySequence getHotkey(const QString &actionName) const;
    QString getAdalightSerialPortName() const;
    int getAdalightSerialPortBaudRate() const;
    QString getArdulightSerialPortName() const;
    int getArdulightSerialPortBaudRate() const;
    bool isConnectedDeviceUsesSerialPort() const;
    // [Adalight | Ardulight | Lightpack | ... | Virtual]
    int getNumberOfLeds(SupportedDevices::DeviceType device) const;
    int getNumberOfConnectedDeviceLeds() const;

protected:
    SettingsProfiles& m_profiles;
};

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

        void setValue(const QString & key, const QVariant& value, bool force = true)
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
    static const SettingsReader * readerInstance() { return instance(); }
    static void Shutdown();

    static QStringList getSupportedDevices();
    static QPoint getDefaultPosition(int ledIndex);
    static QStringList getSupportedSerialPortBaudRates();

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
    QString getLastProfileName() const;
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
    QString getColorSequence(SupportedDevices::DeviceType device) const;
    BaseVersion getVersion() const;

    // Profile
    int getGrabSlowdown() const;
    void setGrabSlowdown(int value);
    bool isBacklightEnabled() const;
    void setIsBacklightEnabled(bool isEnabled);
    bool isGrabAvgColorsEnabled() const;
    void setGrabAvgColorsEnabled(bool isEnabled);
    bool isSendDataOnlyIfColorsChanges() const;
    void setSendDataOnlyIfColorsChanges(bool isEnabled);
    int getLuminosityThreshold() const;
    void setLuminosityThreshold(int value);
    bool isMinimumLuminosityEnabled() const;
    void setMinimumLuminosityEnabled(bool value);
    // [Device]
    int getDeviceRefreshDelay() const;
    void setDeviceRefreshDelay(int value);
    int getDeviceBrightness() const;
    void setDeviceBrightness(int value);
    int getDeviceSmooth() const;
    void setDeviceSmooth(int value);
    int getDeviceColorDepth() const;
    void setDeviceColorDepth(int value);
    double getDeviceGamma() const;
    void setDeviceGamma(double gamma);

    Grab::GrabberType getGrabberType() const;
    void setGrabberType(Grab::GrabberType grabMode);

#ifdef D3D10_GRAB_SUPPORT
    bool isDx1011GrabberEnabled() const;
    void setDx1011GrabberEnabled(bool isEnabled);
#endif

    Lightpack::Mode getLightpackMode();
    void setLightpackMode(Lightpack::Mode mode);
    bool isMoodLampLiquidMode() const;
    void setMoodLampLiquidMode(bool isLiquidMode);
    QColor getMoodLampColor() const;
    void setMoodLampColor(QColor color);
    int getMoodLampSpeed() const;
    void setMoodLampSpeed(int value);

    QList<WBAdjustment> getLedCoefs() const;

    double getLedCoefRed(int ledIndex);
    double getLedCoefGreen(int ledIndex);
    double getLedCoefBlue(int ledIndex);

    void setLedCoefRed(int ledIndex, double value);
    void setLedCoefGreen(int ledIndex, double value);
    void setLedCoefBlue(int ledIndex, double value);

    QSize getLedSize(int ledIndex) const;
    void setLedSize(int ledIndex, QSize size);
    QPoint getLedPosition(int ledIndex) const;
    void setLedPosition(int ledIndex, QPoint position);
    bool isLedEnabled(int ledIndex) const;
    void setLedEnabled(int ledIndex, bool isEnabled);

    uint getLastReadUpdateId() const;
    void setLastReadUpdateId(const uint updateId);

private:
    void setValidLedCoef(int ledIndex, const QString & keyCoef, double coef);
    double getValidLedCoef(int ledIndex, const QString & keyCoef);

    void initDevicesMap();

    void migrateSettings();


public:
    QVariant pluginValue(const QString & pluginId, const QString & key) const;
    void setPluginValue(const QString & pluginId, const QString & key, const QVariant& value);

private:
    // This allows Settings to be deleted with private destructor.
    friend struct QScopedPointerDeleter<Settings>;

    // For testing only.
    friend class ::SettingsTest;

    class TestingOverrides : public Overrides {
    public:
        TestingOverrides() {}
        TestingOverrides& setConnectedDeviceForTests(SupportedDevices::DeviceType deviceType);
        TestingOverrides& setConfigVersionForTests(const BaseVersion& version);
    };

    Settings(const QString & mainConfigPath);
    ~Settings();

    void applyMainProfileOverrides(const Overrides& overrides);
    void applyCurrentProfileOverrides(const Overrides& overrides);
    void verifyMainProfile();
    void verifyCurrentProfile();

    QString m_applicationDirPath; // path to store app generated stuff
    QMap<SupportedDevices::DeviceType, QString> m_devicesTypeToNameMap;
    QMap<SupportedDevices::DeviceType, QString> m_devicesTypeToKeyNumberOfLedsMap;

    static QScopedPointer<Settings> m_instance;
};
} /*SettingsScope*/
