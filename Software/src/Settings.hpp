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

#include "SettingsDefaults.hpp"
#include "enums.hpp"
#include "../common/defs.h"
#include "debug.h"
#include "types.h"

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
	void setValue(const QString & key, const QVariant & value, bool force = true);
	void remove(const QString& key);

	bool beginBatchUpdate();
	void endBatchUpdate();
    void reset();

	struct ScopedBatchUpdateGuard {
		ConfigurationProfile& m_profile;

		ScopedBatchUpdateGuard(ConfigurationProfile& profile)
			: m_profile(profile) { m_profile.beginBatchUpdate(); }

		~ScopedBatchUpdateGuard() { m_profile.endBatchUpdate(); }
	};

private:
	mutable QMutex m_mutex;
	bool m_isInBatchUpdate;
	QString m_name;
	QString m_path;
	QScopedPointer<SettingsSource> m_settings;
};

/*!
  Provides access to persistent settings.
*/
class Settings : public QObject
{
    Q_OBJECT

public:
	class Overrides
	{
	public:
		Overrides() {}

		void setProfile(const QString& profileName);
		void setDebuglevel(Debug::DebugLevels level);
		void apply(ConfigurationProfile& profile) const;

	protected:
		struct OverridingValue {
			QVariant value;
			bool force;
		};

		void setValue(const QString & key, const QVariant& value, bool force = true) {
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
	static Settings * instance() { return m_instance; }

	static QStringList getSupportedDevices();
	static QPoint getDefaultPosition(int ledIndex);
	static QStringList getSupportedSerialPortBaudRates();

	void resetDefaults();
    bool isPresent(const QString & applicationDirPath);

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
	QString getLanguage() const;
	void setLanguage(const QString & language);
	int getDebugLevel() const;
	void setDebugLevel(int debugLvl);
	bool isApiEnabled() const;
	void setIsApiEnabled(bool isEnabled);
	bool isListenOnlyOnLoInterface() const;
	void setListenOnlyOnLoInterface(bool localOnly);
	int getApiPort() const;
	void setApiPort(int apiPort);
	QString getApiAuthKey() const;
	void setApiKey(const QString & apiKey);
	bool isApiAuthEnabled() const;
	void setIsApiAuthEnabled(bool isEnabled);
	bool isExpertModeEnabled() const;
	void setExpertModeEnabled(bool isEnabled);
	bool isKeepLightsOnAfterExit() const;
	void setKeepLightsOnAfterExit(bool isEnabled);
	bool isKeepLightsOnAfterLock() const;
	void setKeepLightsOnAfterLock(bool isEnabled);
	bool isPingDeviceEverySecond() const;
	void setPingDeviceEverySecond(bool isEnabled);
	bool isUpdateFirmwareMessageShown() const;
	void setUpdateFirmwareMessageShown(bool isShown);
	SupportedDevices::DeviceType getConnectedDevice() const;
	void setConnectedDevice(SupportedDevices::DeviceType device);
	QString getConnectedDeviceName() const;
	void setConnectedDeviceName(const QString & deviceName);
	QKeySequence getHotkey(const QString &actionName) const;
	void setHotkey(const QString &actionName, const QKeySequence &keySequence);
	QString getAdalightSerialPortName() const;
	void setAdalightSerialPortName(const QString & port);
	int getAdalightSerialPortBaudRate() const;
	void setAdalightSerialPortBaudRate(const QString & baud);
	QString getArdulightSerialPortName() const;
	void setArdulightSerialPortName(const QString & port);
	int getArdulightSerialPortBaudRate() const;
	void setArdulightSerialPortBaudRate(const QString & baud);
	bool isConnectedDeviceUsesSerialPort() const;
    // [Adalight | Ardulight | Lightpack | ... | Virtual]
	void setNumberOfLeds(SupportedDevices::DeviceType device, int numberOfLeds);
	int getNumberOfLeds(SupportedDevices::DeviceType device) const;
	int getNumberOfConnectedDeviceLeds() const;

	void setColorSequence(SupportedDevices::DeviceType device, QString colorSequence);
	QString getColorSequence(SupportedDevices::DeviceType device) const;

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
	static int getValidDeviceRefreshDelay(int value);
	static int getValidDeviceBrightness(int value);
	static int getValidDeviceSmooth(int value);
	static int getValidDeviceColorDepth(int value);
	static double getValidDeviceGamma(double value);
	static int getValidGrabSlowdown(int value);
	static int getValidMoodLampSpeed(int value);
	static int getValidLuminosityThreshold(int value);

	void setValidLedCoef(int ledIndex, const QString & keyCoef, double coef);
	double getValidLedCoef(int ledIndex, const QString & keyCoef);

	void initDevicesMap();

	void migrateSettings();


public:
	QVariant pluginValue(const QString & pluginId, const QString & key) const;
	void setPluginValue(const QString & pluginId, const QString & key, const QVariant& value);

private:
    // forwarding to m_mainConfig object
	QVariant valueMain(const QString & key) const;
	void setValueMain(const QString & key, const QVariant & value);
    // forwarding to m_currentProfile object
	void setValue(const QString & key, const QVariant & value);
	QVariant value(const QString & key) const;

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

private:
	Settings(const QString & mainConfigPath);
	void applyMainProfileOverrides(const Overrides& overrides);
	void applyCurrentProfileOverrides(const Overrides& overrides);
	void verifyMainProfile();
	void verifyCurrentProfile();

	ConfigurationProfile m_mainProfile;
	ConfigurationProfile m_currentProfile;
	QString m_applicationDirPath; // path to store app generated stuff
	QMap<SupportedDevices::DeviceType, QString> m_devicesTypeToNameMap;
	QMap<SupportedDevices::DeviceType, QString> m_devicesTypeToKeyNumberOfLedsMap;

	static Settings * m_instance;
};
} /*SettingsScope*/
