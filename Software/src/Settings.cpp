/*
 * Settings.cpp
 *
 *  Created on: 22.02.2011
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

#include "Settings.hpp"

#include <QtDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QSize>
#include <QPoint>
#include <QFileInfo>
#include <QDir>
#include <QUuid>

#include "debug.h"
#include "BaseVersion.hpp"

#define MAIN_CONFIG_FILE_VERSION    "4.0"

namespace
{
inline const WBAdjustment getLedAdjustment(size_t ledIndex)
{
    using namespace SettingsScope;

    WBAdjustment wba;
	wba.red = Settings::instance()->getLedCoefRed(ledIndex);
	wba.green = Settings::instance()->getLedCoefGreen(ledIndex);
	wba.blue = Settings::instance()->getLedCoefBlue(ledIndex);
    return wba;
}

struct LedInfo {
	bool isEnabled;
	QPoint position;
	QSize size;
	double wbRed;
	double wbGreen;
	double wbBlue;
};
}

//
// This strings keys and values must be accessible only in current file
//
namespace SettingsScope
{
namespace Main
{
namespace Key
{
// [General]
static const QString MainConfigVersion = "MainConfigVersion";
static const QString ProfileLast = "ProfileLast";
static const QString Language = "Language";
static const QString DebugLevel = "DebugLevel";
static const QString IsExpertModeEnabled = "IsExpertModeEnabled";
static const QString IsKeepLightsOnAfterExit = "IsKeepLightsOnAfterExit";
static const QString IsKeepLightsOnAfterLock = "IsKeepLightsOnAfterLock";
static const QString IsPingDeviceEverySecond = "IsPingDeviceEverySecond";
static const QString IsUpdateFirmwareMessageShown = "IsUpdateFirmwareMessageShown";
static const QString ConnectedDevice = "ConnectedDevice";
static const QString SupportedDevices = "SupportedDevices";
static const QString LastReadUpdateId = "LastReadUpdateId";

// [Hotkeys]
namespace Hotkeys
{
static const QString SettingsPrefix = "HotKeys/";
}

// [API]
namespace Api
{
static const QString IsEnabled = "API/IsEnabled";
static const QString ListenOnlyOnLoInterface = "API/ListenOnlyOnLoInterface";
static const QString Port = "API/Port";
static const QString AuthKey = "API/AuthKey";
}
namespace Adalight
{
static const QString NumberOfLeds = "Adalight/NumberOfLeds";
static const QString ColorSequence = "Adalight/ColorSequence";
static const QString Port = "Adalight/SerialPort";
static const QString BaudRate = "Adalight/BaudRate";
}
namespace Ardulight
{
static const QString NumberOfLeds = "Ardulight/NumberOfLeds";
static const QString ColorSequence = "Ardulight/ColorSequence";
static const QString Port = "Ardulight/SerialPort";
static const QString BaudRate = "Ardulight/BaudRate";
}
namespace AlienFx
{
static const QString NumberOfLeds = "AlienFx/NumberOfLeds";
}
namespace Lightpack
{
static const QString NumberOfLeds = "Lightpack/NumberOfLeds";
}
namespace Virtual
{
static const QString NumberOfLeds = "Virtual/NumberOfLeds";
}
} /*Key*/

namespace Value
{
static const QString MainConfigVersion = MAIN_CONFIG_FILE_VERSION;

namespace ConnectedDevice
{
static const QString LightpackDevice = "Lightpack";
static const QString AlienFxDevice = "AlienFx";
static const QString AdalightDevice = "Adalight";
static const QString ArdulightDevice = "Ardulight";
static const QString VirtualDevice = "Virtual";
}

} /*Value*/
} /*Main*/

namespace Profile
{
namespace Key
{
// [General]
static const QString LightpackMode = "LightpackMode";
static const QString IsBacklightEnabled = "IsBacklightEnabled";
// [Grab]
namespace Grab
{
static const QString Grabber = "Grab/Grabber";
static const QString IsAvgColorsEnabled = "Grab/IsAvgColorsEnabled";
static const QString IsSendDataOnlyIfColorsChanges = "Grab/IsSendDataOnlyIfColorsChanges";
static const QString Slowdown = "Grab/Slowdown";
static const QString LuminosityThreshold = "Grab/LuminosityThreshold";
static const QString IsMinimumLuminosityEnabled = "Grab/IsMinimumLuminosityEnabled";
static const QString IsDx1011GrabberEnabled = "Grab/IsDX1011GrabberEnabled";
}
// [MoodLamp]
namespace MoodLamp
{
static const QString IsLiquidMode = "MoodLamp/LiquidMode";
static const QString Color = "MoodLamp/Color";
static const QString Speed = "MoodLamp/Speed";
}
// [Device]
namespace Device
{
static const QString RefreshDelay = "Device/RefreshDelay";
static const QString Smooth = "Device/Smooth";
static const QString Brightness = "Device/Brightness";
static const QString ColorDepth = "Device/ColorDepth";
static const QString Gamma = "Device/Gamma";
}
// [LED_i]
namespace Led
{

static const QString Prefix = "LED_";
static const QString IsEnabled = "IsEnabled";
static const QString Size = "Size";
static const QString Position = "Position";
static const QString CoefRed = "CoefRed";
static const QString CoefGreen = "CoefGreen";
static const QString CoefBlue = "CoefBlue";
}
} /*Key*/

namespace Value
{

namespace LightpackMode
{
static const QString Ambilight = "Ambilight";
static const QString MoodLamp = "MoodLamp";
}

namespace GrabberType
{
static const QString Qt = "Qt";
static const QString QtEachWidget = "QtEachWidget";
static const QString WinAPI = "WinAPI";
static const QString WinAPIEachWidget = "WinAPIEachWidget";
static const QString X11 = "X11";
static const QString D3D9 = "D3D9";
static const QString MacCoreGraphics = "MacCoreGraphics";
}

} /*Value*/
} /*Profile*/

namespace
{
class QSettingsSource : public SettingsSource
{
public:
	QSettingsSource(const QString& path) : m_settings(path, QSettings::IniFormat)
	{
		m_settings.setIniCodec("UTF-8");
		DEBUG_LOW_LEVEL << "Settings file:" << m_settings.fileName();
	}

	virtual QVariant value(const QString & key) const
	{
		return m_settings.value(key);
	}

	virtual void setValue(const QString & key, const QVariant & value)
	{
		m_settings.setValue(key, value);
	}

	virtual bool contains(const QString& key) const
	{
		return m_settings.contains(key);
	}

	virtual void remove(const QString& key) {
		m_settings.remove(key);
	}

	virtual void sync() { m_settings.sync(); }

private:
	QSettings m_settings;
};

class MainProfileOverrides : public Settings::Overrides {
public:
	MainProfileOverrides() {
		setValue(Main::Key::MainConfigVersion,      Main::Value::MainConfigVersion /* rewrite */);
		setValue(Main::Key::ProfileLast,            Main::ProfileNameDefault);
		setValue(Main::Key::Language,               Main::LanguageDefault);
		setValue(Main::Key::DebugLevel,             Main::DebugLevelDefault);
		setValue(Main::Key::IsExpertModeEnabled,    Main::IsExpertModeEnabledDefault);
		setValue(Main::Key::IsKeepLightsOnAfterExit,   Main::IsKeepLightsOnAfterExit);
		setValue(Main::Key::IsKeepLightsOnAfterLock, Main::IsKeepLightsOnAfterLock);
		setValue(Main::Key::IsPingDeviceEverySecond,Main::IsPingDeviceEverySecond);
		setValue(Main::Key::IsUpdateFirmwareMessageShown, Main::IsUpdateFirmwareMessageShown);
		setValue(Main::Key::ConnectedDevice,        Main::ConnectedDeviceDefault);
		setValue(Main::Key::SupportedDevices,       Main::SupportedDevices, true /* always rewrite this information to main config */);
		setValue(Main::Key::Api::IsEnabled,         Main::Api::IsEnabledDefault);
		setValue(Main::Key::Api::ListenOnlyOnLoInterface, Main::Api::ListenOnlyOnLoInterfaceDefault);
		setValue(Main::Key::Api::Port,              Main::Api::PortDefault);

		// Generation AuthKey as new UUID
		setValue(Main::Key::Api::AuthKey,           Main::Api::AuthKey);

		// Serial device configuration
		setValue(Main::Key::Adalight::Port,             Main::Adalight::PortDefault);
		setValue(Main::Key::Adalight::BaudRate,         Main::Adalight::BaudRateDefault);
		setValue(Main::Key::Adalight::NumberOfLeds,     Main::Adalight::NumberOfLedsDefault);
		setValue(Main::Key::Adalight::ColorSequence,    Main::Adalight::ColorSequence);

		setValue(Main::Key::Ardulight::Port,            Main::Ardulight::PortDefault);
		setValue(Main::Key::Ardulight::BaudRate,        Main::Ardulight::BaudRateDefault);
		setValue(Main::Key::Ardulight::NumberOfLeds,    Main::Ardulight::NumberOfLedsDefault);
		setValue(Main::Key::Ardulight::ColorSequence,   Main::Ardulight::ColorSequence);

		setValue(Main::Key::AlienFx::NumberOfLeds,      Main::AlienFx::NumberOfLedsDefault);
		setValue(Main::Key::Lightpack::NumberOfLeds,    Main::Lightpack::NumberOfLedsDefault);
		setValue(Main::Key::Virtual::NumberOfLeds,      Main::Virtual::NumberOfLedsDefault);
		setValue(Main::Key::LastReadUpdateId,           Main::LastReadUpdateId);
	}
};

class CurrentProfileOverrides : public Settings::Overrides {
public:
	CurrentProfileOverrides(bool resetDefault) {
		DEBUG_LOW_LEVEL << Q_FUNC_INFO << resetDefault;

		// [General]
		setValue(Profile::Key::LightpackMode, Profile::LightpackModeDefault, resetDefault);
		setValue(Profile::Key::IsBacklightEnabled, Profile::IsBacklightEnabledDefault, resetDefault);
		// [Grab]
		setValue(Profile::Key::Grab::Grabber,       Profile::Grab::GrabberDefaultString, resetDefault);
		setValue(Profile::Key::Grab::IsAvgColorsEnabled, Profile::Grab::IsAvgColorsEnabledDefault, resetDefault);
		setValue(Profile::Key::Grab::IsSendDataOnlyIfColorsChanges, Profile::Grab::IsSendDataOnlyIfColorsChangesDefault, resetDefault);
		setValue(Profile::Key::Grab::Slowdown,      Profile::Grab::SlowdownDefault, resetDefault);
		setValue(Profile::Key::Grab::LuminosityThreshold, Profile::Grab::MinimumLevelOfSensitivityDefault, resetDefault);
		setValue(Profile::Key::Grab::IsMinimumLuminosityEnabled, Profile::Grab::IsMinimumLuminosityEnabledDefault, resetDefault);
		// [MoodLamp]
		setValue(Profile::Key::MoodLamp::IsLiquidMode,  Profile::MoodLamp::IsLiquidMode, resetDefault);
		setValue(Profile::Key::MoodLamp::Color,         Profile::MoodLamp::ColorDefault, resetDefault);
		setValue(Profile::Key::MoodLamp::Speed,         Profile::MoodLamp::SpeedDefault, resetDefault);
		// [Device]
		setValue(Profile::Key::Device::RefreshDelay,Profile::Device::RefreshDelayDefault, resetDefault);
		setValue(Profile::Key::Device::Brightness,  Profile::Device::BrightnessDefault, resetDefault);
		setValue(Profile::Key::Device::Smooth,      Profile::Device::SmoothDefault, resetDefault);
		setValue(Profile::Key::Device::Gamma,       Profile::Device::GammaDefault, resetDefault);
		setValue(Profile::Key::Device::ColorDepth,  Profile::Device::ColorDepthDefault, resetDefault);

		for (int i = 0; i < MaximumNumberOfLeds::AbsoluteMaximum; i++)
		{
			const QPoint ledPosition = Settings::getDefaultPosition(i);
			const QString prefix = Profile::Key::Led::Prefix + QString::number(i + 1) + "/";

			setValue(prefix + Profile::Key::Led::IsEnabled, Profile::Led::IsEnabledDefault, resetDefault);
			setValue(prefix + Profile::Key::Led::Position,	ledPosition, resetDefault);
			setValue(prefix + Profile::Key::Led::Size,		Profile::Led::SizeDefault, resetDefault);
			setValue(prefix + Profile::Key::Led::CoefRed,	Profile::Led::CoefDefault, resetDefault);
			setValue(prefix + Profile::Key::Led::CoefGreen, Profile::Led::CoefDefault, resetDefault);
			setValue(prefix + Profile::Key::Led::CoefBlue,	Profile::Led::CoefDefault, resetDefault);
		}

		DEBUG_LOW_LEVEL << Q_FUNC_INFO << "led";
	}
};

struct ConditionalMutexLocker {
	QMutex& m_mutex;
	const bool m_shouldLock;

	ConditionalMutexLocker(QMutex& mutex, bool lock)
		: m_mutex(mutex), m_shouldLock(lock) {
		if (lock)
			m_mutex.lock();
	}

	~ConditionalMutexLocker() { if (m_shouldLock) m_mutex.unlock(); }
};

template <typename T>
static inline bool between(T value, T min, T max) {
	return value >= min && value <= max;
}

template <typename T>
static inline bool between_exclusive(T value, T min, T max) {
	return value > min && value < max;
}

template <typename T>
inline T clamp_value(T value, const T minInclusive, const T maxInclusive) {
	if (value < minInclusive)
		value = minInclusive;
	else if (value > maxInclusive)
		value = maxInclusive;
	return value;
}

static SettingsSource* (*g_settingsSourceFabric)(const QString& path) = NULL;
}

ConfigurationProfile::ConfigurationProfile() : m_isInBatchUpdate(false) {

}

bool ConfigurationProfile::init(const QString& path, const QString& name)
{
	if (g_settingsSourceFabric) {
		m_settings.reset(g_settingsSourceFabric(path));
	}
	else {
		m_settings.reset(new QSettingsSource(path));
	}
	m_name = name;
	m_path = path;
	return isInitialized();
}

QVariant ConfigurationProfile::value(const QString & key) const {
	if (!m_settings) {
		qWarning() << Q_FUNC_INFO << "m_settings == NULL";
		return QVariant();
	}

	QVariant value;
	{
		ConditionalMutexLocker locker(m_mutex, !m_isInBatchUpdate);
		value = m_settings->value(key);
	}
	DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
	return value;
}

QVariant ConfigurationProfile::valueOrDefault(const QString & key, const QVariant& defaultValue) const {
	if (!m_settings) {
		qWarning() << Q_FUNC_INFO << "m_settings == NULL";
		return defaultValue;
	}

	QVariant value = defaultValue;
	{
		ConditionalMutexLocker locker(m_mutex, !m_isInBatchUpdate);
		if (m_settings->contains(key))
			value = m_settings->value(key);
	}
	DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
	return value;
}

void ConfigurationProfile::setValue(const QString & key, const QVariant & value, bool force) {
	if (!m_settings) {
		qWarning() << "ConfigurationProfile wasn't initialized.";
		return;
	}

	ConditionalMutexLocker locker(m_mutex, !m_isInBatchUpdate);
	if (force) {
		m_settings->setValue(key, value);
	} else {
		if (!m_settings->contains(key)) {
			DEBUG_LOW_LEVEL << "Settings:"<< key << "not found."
					<< "Set it to default value: " << value.toString();

			m_settings->setValue(key, value);
		}
		// else option exists do nothing
	}
}

void ConfigurationProfile::remove(const QString& key) {
	Q_ASSERT(isInitialized());
	if (!isInitialized())
		return;

	m_settings->remove(key);
}

bool ConfigurationProfile::beginBatchUpdate() {
	Q_ASSERT(!m_isInBatchUpdate);
	if (m_isInBatchUpdate)
		return false;
	m_mutex.lock();
	m_isInBatchUpdate = true;
	return true;
}

void ConfigurationProfile::endBatchUpdate() {
	Q_ASSERT(m_isInBatchUpdate);
	m_mutex.unlock();
	m_settings->sync();
	m_isInBatchUpdate = false;
}

void ConfigurationProfile::reset() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    if (!isInitialized())
        return;

    Q_ASSERT(!m_isInBatchUpdate);
    m_settings->sync();
    m_name.clear();
    m_path.clear();
    m_settings.reset();
}

void Settings::Overrides::setProfile(const QString& profileName)
{
	setValue(Main::Key::ProfileLast, profileName);
}

void Settings::Overrides::setDebuglevel(Debug::DebugLevels level)
{
	setValue(Main::Key::DebugLevel, QVariant(static_cast<uint>(level)));
}

void Settings::Overrides::apply(ConfigurationProfile& profile) const
{
	ConfigurationProfile::ScopedBatchUpdateGuard guard(profile);
	for (OverridesMap::const_iterator it = m_overrides.cbegin(); it != m_overrides.cend(); ++it)
	{
		const OverridingValue& value = it.value();
		//DEBUG_LOW_LEVEL << "Overriding setting[" << it.key() << "] with value = " << value.value;
		profile.setValue(it.key(), value.value, value.force);
	}
}

Settings * Settings::m_instance = NULL;

Settings::Settings(const QString& applicationDirPath)
	: QObject(NULL) {
	const QDir applicationDir(applicationDirPath);
	Q_ASSERT(applicationDir.exists());

	m_applicationDirPath = applicationDir.absolutePath() + "/";
	DEBUG_LOW_LEVEL << "Settings applicationDirPath = " << m_applicationDirPath;
	m_mainProfile.init(applicationDir.absoluteFilePath("main.conf"), "main");

	const QString profileName = m_mainProfile.valueOrDefault(Main::Key::ProfileLast, Main::ProfileNameDefault).toString();
	DEBUG_LOW_LEVEL << "Current profile name = " << profileName;
	m_currentProfile.init(applicationDir.absoluteFilePath(profileName + ".ini"), profileName);

    qRegisterMetaType<Grab::GrabberType>("Grab::GrabberType");
    qRegisterMetaType<QColor>("QColor");
    qRegisterMetaType<SupportedDevices::DeviceType>("SupportedDevices::DeviceType");
    qRegisterMetaType<Lightpack::Mode>("Lightpack::Mode");
}

void Settings::applyMainProfileOverrides(const Overrides& overrides) {
	overrides.apply(m_mainProfile);
}

void Settings::applyCurrentProfileOverrides(const Overrides& overrides) {
	overrides.apply(m_currentProfile);

	this->currentProfileInited(getCurrentProfileName());
}

// Desktop should be initialized before call Settings::Initialize()
// static
bool Settings::Initialize(const QString & applicationDirPath,
                          const Settings::Overrides& overrides)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	if (m_instance != NULL)
        return true;

	const QDir applicationDir(applicationDirPath);
	Q_ASSERT(applicationDir.exists());

	QString mainConfigPath = applicationDir.absoluteFilePath("main.conf");
    bool settingsWasPresent = QFileInfo(mainConfigPath).exists();

	m_instance = new Settings(applicationDirPath);
	m_instance->applyMainProfileOverrides(MainProfileOverrides());
	m_instance->applyMainProfileOverrides(overrides);

    bool ok = false;
	const uint debugLevel = m_instance->m_mainProfile.value(Main::Key::DebugLevel).toUInt(&ok);

	if (ok && debugLevel <= Debug::HighLevel)
    {
		g_debugLevel = debugLevel;
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "debugLevel =" << g_debugLevel;
    } else {
        qWarning() << "DebugLevel in config has an invalid value, set the default" << Main::DebugLevelDefault;
		m_instance->m_mainProfile.setValue(Main::Key::DebugLevel, Main::DebugLevelDefault);
        g_debugLevel = Main::DebugLevelDefault;
    }

    // Initialize m_devicesMap for mapping DeviceType on DeviceName
	m_instance->initDevicesMap();

    // Initialize profile with default values without reset exists values
	m_instance->applyCurrentProfileOverrides(CurrentProfileOverrides(false));

    // Do changes to settings if necessary
	m_instance->migrateSettings();

	// Verify initial settings.
	m_instance->verifyMainProfile();
	m_instance->verifyCurrentProfile();

    return settingsWasPresent;
}

//
//  Set all settings in current config to default values
//
void Settings::resetDefaults()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	applyCurrentProfileOverrides(CurrentProfileOverrides(true /* = reset to default values */));
}

QStringList Settings::findAllProfiles() const
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	QFileInfo setsFile(getProfilesPath());
    QFileInfoList iniFiles = setsFile.absoluteDir().entryInfoList(QStringList("*.ini"));

    QStringList settingsFiles;
    for(int i=0; i<iniFiles.count(); i++){
        QString compBaseName = iniFiles.at(i).completeBaseName();
        settingsFiles.append(compBaseName);
    }

    return settingsFiles;
}

void Settings::loadOrCreateProfile(const QString & profileName)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << profileName;

	if (m_currentProfile.isInitialized() && m_currentProfile.name() == profileName)
        return; //nothing to change, profile is already loaded

    const QString profileNewPath = getProfilesPath() + profileName + ".ini";
    if (m_currentProfile.isInitialized()) {
        // Copy current settings to new one
        QFile::copy(m_currentProfile.path(), profileNewPath);
    }

    if (m_currentProfile.init(profileNewPath, profileName)) {
        // Initialize profile with default values without reset exists values
        m_instance->applyCurrentProfileOverrides(CurrentProfileOverrides(false));

        // Verify initial settings.
        m_instance->verifyCurrentProfile();

        DEBUG_LOW_LEVEL << "Settings file:" << m_currentProfile.path();
        m_mainProfile.setValue(Main::Key::ProfileLast, profileName);
        this->profileLoaded(profileName);
    }
}

void Settings::renameCurrentProfile(const QString & profileName)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "from" << getCurrentProfileName() << "to" << profileName;

	// TODO: implement.
	Q_ASSERT(false);

    if (!m_currentProfile.isInitialized())
    {
        qWarning() << "void Settings::renameCurrentConfig(): fail, m_currentProfile not initialized";
        return;
    }

    if (m_currentProfile.name() == profileName)
        return;

    // Rename current settings to new one
    const QString profileNewPath = getProfilesPath() + profileName + ".ini";
    QFile::rename(m_currentProfile.path(), profileNewPath);

    if (m_currentProfile.init(profileNewPath, profileName)) {
        // Initialize profile with default values without reset exists values
        m_instance->applyCurrentProfileOverrides(CurrentProfileOverrides(false));

        // Verify initial settings.
        m_instance->verifyCurrentProfile();

        DEBUG_LOW_LEVEL << "Settings file renamed:" << m_currentProfile.path();

        m_mainProfile.setValue(Main::Key::ProfileLast, profileName);
        this->currentProfileNameChanged(profileName);
    }
}

void Settings::removeCurrentProfile()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    if (!m_currentProfile.isInitialized())
    {
        qWarning() << Q_FUNC_INFO << "current profile not loaded, nothing to remove";
        m_mainProfile.setValue(Main::Key::ProfileLast, Main::ProfileNameDefault);
        return;
    }

    if (QFile::remove( m_currentProfile.path() ) == false)
    {
        qWarning() << Q_FUNC_INFO << "QFile::remove(" << m_currentProfile.path() << ") fail";
        return;
    }

    m_currentProfile.reset();
	m_mainProfile.setValue(Main::Key::ProfileLast, Main::ProfileNameDefault);
	this->currentProfileRemoved();
}


QString Settings::getCurrentProfileName() const
{
	if (m_currentProfile.isInitialized()) {
		DEBUG_MID_LEVEL << Q_FUNC_INFO << m_currentProfile.name();
    } else {
        qCritical() << Q_FUNC_INFO << ("current profile isn't set!!!");
    }

	return m_currentProfile.name();
}

QString Settings::getCurrentProfilePath() const
{
	if (m_currentProfile.isInitialized()) {
		DEBUG_MID_LEVEL << Q_FUNC_INFO << m_currentProfile.path();
    } else {
        qCritical() << Q_FUNC_INFO << ("current profile isn't set!!!");
    }

	return m_currentProfile.path();
}

bool Settings::isProfileLoaded() const
{
    DEBUG_HIGH_LEVEL << Q_FUNC_INFO;
	return m_currentProfile.isInitialized();
}

QString Settings::getApplicationDirPath() const
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << m_applicationDirPath;
    return m_applicationDirPath;
}

QString Settings::getMainConfigPath() const
{
    QString mainConfPath = m_applicationDirPath + "main.conf";
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << mainConfPath;
    return mainConfPath;
}

// static
QPoint Settings::getDefaultPosition(int ledIndex)
{
    QPoint result;

    if (ledIndex > (MaximumNumberOfLeds::Default - 1))
    {
        int x = (ledIndex - MaximumNumberOfLeds::Default) * 10 /* px */;
        return QPoint(x, 0);
    }

    QRect screen = QApplication::desktop()->screenGeometry();

    int ledsCountDiv2 = MaximumNumberOfLeds::Default / 2;

    if (ledIndex < ledsCountDiv2)
    {
        result.setX(0);
    } else {
        result.setX(screen.width() - Profile::Led::SizeDefault.width());
    }

    int height = ledsCountDiv2 * Profile::Led::SizeDefault.height();

    int y = screen.height() / 2 - height / 2;

    result.setY(y + (ledIndex % ledsCountDiv2) * Profile::Led::SizeDefault.height());

    return result;
}

QString Settings::getLastProfileName() const
{
	return m_mainProfile.value(Main::Key::ProfileLast).toString();
}

QString Settings::getLanguage() const
{
	return m_mainProfile.value(Main::Key::Language).toString();
}

void Settings::setLanguage(const QString & language)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::Language, language);
	this->languageChanged(language);
}

int Settings::getDebugLevel() const
{
	return m_mainProfile.value(Main::Key::DebugLevel).toInt();
}

void Settings::setDebugLevel(int debugLvl)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::DebugLevel, debugLvl);
	this->debugLevelChanged(debugLvl);
}

bool Settings::isApiEnabled() const
{
	return m_mainProfile.value(Main::Key::Api::IsEnabled).toBool();
}

void Settings::setIsApiEnabled(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::Api::IsEnabled, isEnabled);
	this->apiServerSettingsChanged();
}

bool Settings::isListenOnlyOnLoInterface() const
{
	return m_mainProfile.value(Main::Key::Api::ListenOnlyOnLoInterface).toBool();
}

void Settings::setListenOnlyOnLoInterface(bool localOnly)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::Api::ListenOnlyOnLoInterface, localOnly);
	this->apiServerSettingsChanged();
}

int Settings::getApiPort() const
{
	return m_mainProfile.value(Main::Key::Api::Port).toInt();
}

void Settings::setApiPort(int apiPort)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::Api::Port, apiPort);
	this->apiServerSettingsChanged();
}

QString Settings::getApiAuthKey() const
{
	return m_mainProfile.value(Main::Key::Api::AuthKey).toString();
}

void Settings::setApiKey(const QString & apiKey)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::Api::AuthKey, apiKey);
	this->apiKeyChanged(apiKey);
}

bool Settings::isApiAuthEnabled() const
{
    return !getApiAuthKey().isEmpty();
}

bool Settings::isExpertModeEnabled() const
{   
	return m_mainProfile.value(Main::Key::IsExpertModeEnabled).toBool();
}

void Settings::setExpertModeEnabled(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::IsExpertModeEnabled, isEnabled);
	this->expertModeEnabledChanged(isEnabled);
}

bool Settings::isKeepLightsOnAfterExit() const
{
	return m_mainProfile.value(Main::Key::IsKeepLightsOnAfterExit).toBool();
}

void Settings::setKeepLightsOnAfterExit(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::IsKeepLightsOnAfterExit, isEnabled);
	this->keepLightsOnAfterExitChanged(isEnabled);
}

bool Settings::isKeepLightsOnAfterLock() const
{
	return m_mainProfile.value(Main::Key::IsKeepLightsOnAfterLock).toBool();
}

void Settings::setKeepLightsOnAfterLock(bool isEnabled)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::IsKeepLightsOnAfterLock, isEnabled);
	this->keepLightsOnAfterLockChanged(isEnabled);
}

bool Settings::isPingDeviceEverySecond() const
{
	return m_mainProfile.value(Main::Key::IsPingDeviceEverySecond).toBool();
}

void Settings::setPingDeviceEverySecond(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::IsPingDeviceEverySecond, isEnabled);
	this->pingDeviceEverySecondEnabledChanged(isEnabled);
}

bool Settings::isUpdateFirmwareMessageShown() const
{
	return m_mainProfile.value(Main::Key::IsUpdateFirmwareMessageShown).toBool();
}

void Settings::setUpdateFirmwareMessageShown(bool isShown)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::IsUpdateFirmwareMessageShown, isShown);
	this->updateFirmwareMessageShownChanged(isShown);
}

void Settings::verifyMainProfile() {
	const QString deviceName = m_mainProfile.value(Main::Key::ConnectedDevice).toString();
	if (!m_devicesTypeToNameMap.values().contains(deviceName))
	{
		qWarning() << Q_FUNC_INFO
				   << Main::Key::ConnectedDevice
				   << "in main config contains crap or unsupported device,"
				   << "reset it to default value:" << Main::ConnectedDeviceDefault;

		setConnectedDevice(SupportedDevices::DefaultDeviceType);
	}
}

void Settings::verifyCurrentProfile() {

}

SupportedDevices::DeviceType Settings::getConnectedDevice() const
{
	const QString deviceName = m_mainProfile.value(Main::Key::ConnectedDevice).toString();
    return m_devicesTypeToNameMap.key(deviceName, SupportedDevices::DefaultDeviceType);
}

void Settings::setConnectedDevice(SupportedDevices::DeviceType device)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	const QString deviceName = m_devicesTypeToNameMap.value(device, Main::ConnectedDeviceDefault);

	m_mainProfile.setValue(Main::Key::ConnectedDevice, deviceName);
	this->connectedDeviceChanged(device);
}

QString Settings::getConnectedDeviceName() const
{
    return m_devicesTypeToNameMap.value(getConnectedDevice(), Main::ConnectedDeviceDefault);
}

void Settings::setConnectedDeviceName(const QString & deviceName)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	if (deviceName.isEmpty())
        return; // silent return

    if (m_devicesTypeToNameMap.values().contains(deviceName) == false)
    {
        qCritical() << Q_FUNC_INFO << "Failure during check the device name" << deviceName << "in m_devicesMap. The main config has not changed.";
        return;
    }

	m_mainProfile.setValue(Main::Key::ConnectedDevice, deviceName);
	this->connectedDeviceChanged(m_devicesTypeToNameMap.key(deviceName));
}

// static
QStringList Settings::getSupportedDevices()
{
    return Main::SupportedDevices.split(',');
}

QKeySequence Settings::getHotkey(const QString &actionName) const
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	const QString key = Main::Key::Hotkeys::SettingsPrefix + actionName;
	const QVariant& keyValue = m_mainProfile.valueOrDefault(key, QVariant());
	return keyValue.isNull() ? QKeySequence() : QKeySequence(keyValue.toString());
}

void Settings::setHotkey(const QString &actionName, const QKeySequence &keySequence)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	const QString key = Main::Key::Hotkeys::SettingsPrefix + actionName;
    QKeySequence oldKeySequence = getHotkey(actionName);
    if( keySequence.isEmpty() ) {
		m_mainProfile.setValue(key, Main::HotKeys::HotkeyDefault);
    } else {
		m_mainProfile.setValue(key, keySequence.toString());
    }
	this->hotkeyChanged(actionName, keySequence, oldKeySequence);
}

QString Settings::getAdalightSerialPortName() const
{
	return m_mainProfile.value(Main::Key::Adalight::Port).toString();
}

void Settings::setAdalightSerialPortName(const QString & port)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::Adalight::Port, port);
	this->adalightSerialPortNameChanged(port);
}

int Settings::getAdalightSerialPortBaudRate() const
{
    // TODO: validate baudrate reading from settings file
	return m_mainProfile.value(Main::Key::Adalight::BaudRate).toInt();
}

void Settings::setAdalightSerialPortBaudRate(const QString & baud)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    // TODO: validator
	m_mainProfile.setValue(Main::Key::Adalight::BaudRate, baud);
	this->adalightSerialPortBaudRateChanged(baud);
}

QString Settings::getArdulightSerialPortName() const
{
	return m_mainProfile.value(Main::Key::Ardulight::Port).toString();
}

void Settings::setArdulightSerialPortName(const QString & port)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_mainProfile.setValue(Main::Key::Ardulight::Port, port);
	this->ardulightSerialPortNameChanged(port);
}

int Settings::getArdulightSerialPortBaudRate() const
{
    // TODO: validate baudrate reading from settings file
	return m_mainProfile.value(Main::Key::Ardulight::BaudRate).toInt();
}

void Settings::setArdulightSerialPortBaudRate(const QString & baud)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    // TODO: validator
	m_mainProfile.setValue(Main::Key::Ardulight::BaudRate, baud);
	this->ardulightSerialPortBaudRateChanged(baud);
}

// static
QStringList Settings::getSupportedSerialPortBaudRates()
{
    QStringList list;

    // TODO: Add more baud rates if need it
    list.append("115200");
    list.append("57600");
    list.append("9600");
    return list;
}

bool Settings::isConnectedDeviceUsesSerialPort() const
{
    switch (getConnectedDevice())
    {
    case SupportedDevices::DeviceTypeAdalight:
        return true;
    case SupportedDevices::DeviceTypeArdulight:
        return true;
    default:
        return false;
    }
}

void Settings::setNumberOfLeds(SupportedDevices::DeviceType device, int numberOfLeds)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    if(getNumberOfLeds(device) == numberOfLeds)
        //nothing to do
        return;

	const QString key = m_devicesTypeToKeyNumberOfLedsMap.value(device);

	if (key.isEmpty())
    {
        qCritical() << Q_FUNC_INFO << "Device type not recognized, device ==" << device << "numberOfLeds ==" << numberOfLeds;
        return;
    }

	m_mainProfile.setValue(key, numberOfLeds);
    {
        using namespace SupportedDevices;
        switch(device) {
            case DeviceTypeLightpack:
			this->lightpackNumberOfLedsChanged(numberOfLeds);
            break;

            case DeviceTypeAdalight:
			this->adalightNumberOfLedsChanged(numberOfLeds);
            break;

            case DeviceTypeArdulight:
			this->ardulightNumberOfLedsChanged(numberOfLeds);
            break;

            case DeviceTypeVirtual:
			this->virtualNumberOfLedsChanged(numberOfLeds);
            break;
        default:
            qCritical() << Q_FUNC_INFO << "Device type not recognized, device ==" << device << "numberOfLeds ==" << numberOfLeds;
        }
    }
}

int Settings::getNumberOfLeds(SupportedDevices::DeviceType device) const
{
	const QString key = m_devicesTypeToKeyNumberOfLedsMap.value(device);

	if (key.isEmpty())
    {
        qCritical() << Q_FUNC_INFO << "Device type not recognized, device ==" << device;
        return MaximumNumberOfLeds::Default;
    }

    // TODO: validator on maximum number of leds for current 'device'
	return m_mainProfile.value(key).toInt();
}

void Settings::setColorSequence(SupportedDevices::DeviceType device, QString colorSequence)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << device << colorSequence;
    switch (device)
    {
        case  SupportedDevices::DeviceTypeAdalight:
			m_mainProfile.setValue(Main::Key::Adalight::ColorSequence, colorSequence);
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "ada" << colorSequence;
            break;
        case SupportedDevices::DeviceTypeArdulight:
			m_mainProfile.setValue(Main::Key::Ardulight::ColorSequence, colorSequence);
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "ard" << colorSequence;
            break;
        default:
            qWarning() << Q_FUNC_INFO << "Unsupported device type: " << device << m_devicesTypeToNameMap.value(device);
            return;
    }
	this->deviceColorSequenceChanged(colorSequence);
}

QString Settings::getColorSequence(SupportedDevices::DeviceType device) const
{
    switch (device)
    {
        case SupportedDevices::DeviceTypeAdalight:
			return m_mainProfile.value(Main::Key::Adalight::ColorSequence).toString();
            break;
        case SupportedDevices::DeviceTypeArdulight:
			return m_mainProfile.value(Main::Key::Ardulight::ColorSequence).toString();
            break;
        default:
            qWarning() << Q_FUNC_INFO << "Unsupported device type: " << device << m_devicesTypeToNameMap.value(device);
    }
    return NULL;
}

int Settings::getGrabSlowdown() const
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    return getValidGrabSlowdown(value(Profile::Key::Grab::Slowdown).toInt());
}

void Settings::setGrabSlowdown(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Grab::Slowdown, getValidGrabSlowdown(value));
	this->grabSlowdownChanged(value);
}

bool Settings::isBacklightEnabled() const
{
	return m_currentProfile.value(Profile::Key::IsBacklightEnabled).toBool();
}

void Settings::setIsBacklightEnabled(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::IsBacklightEnabled, isEnabled);
	this->backlightEnabledChanged(isEnabled);
}

bool Settings::isGrabAvgColorsEnabled() const
{
	return m_currentProfile.value(Profile::Key::Grab::IsAvgColorsEnabled).toBool();
}

void Settings::setGrabAvgColorsEnabled(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Grab::IsAvgColorsEnabled, isEnabled);
	this->grabAvgColorsEnabledChanged(isEnabled);
}

bool Settings::isSendDataOnlyIfColorsChanges() const
{
	return m_currentProfile.value(Profile::Key::Grab::IsSendDataOnlyIfColorsChanges).toBool();
}

void Settings::setSendDataOnlyIfColorsChanges(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Grab::IsSendDataOnlyIfColorsChanges, isEnabled);
	this->sendDataOnlyIfColorsChangesChanged(isEnabled);
}

int Settings::getLuminosityThreshold() const
{
	return m_currentProfile.value(Profile::Key::Grab::LuminosityThreshold).toInt();
}

void Settings::setLuminosityThreshold(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Grab::LuminosityThreshold, getValidLuminosityThreshold(value));
	this->luminosityThresholdChanged(value);
}

bool Settings::isMinimumLuminosityEnabled() const
{
	return m_currentProfile.value(Profile::Key::Grab::IsMinimumLuminosityEnabled).toBool();
}

void Settings::setMinimumLuminosityEnabled(bool value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Grab::IsMinimumLuminosityEnabled, value);
	this->minimumLuminosityEnabledChanged(value);
}

int Settings::getDeviceRefreshDelay() const
{
    return getValidDeviceRefreshDelay(value(Profile::Key::Device::RefreshDelay).toInt());
}

void Settings::setDeviceRefreshDelay(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Device::RefreshDelay, getValidDeviceRefreshDelay(value));
	this->deviceRefreshDelayChanged(value);
}

int Settings::getDeviceBrightness() const
{
    return getValidDeviceBrightness(value(Profile::Key::Device::Brightness).toInt());
}

void Settings::setDeviceBrightness(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Device::Brightness, getValidDeviceBrightness(value));
	this->deviceBrightnessChanged(value);
}

int Settings::getDeviceSmooth() const
{
    return getValidDeviceSmooth(value(Profile::Key::Device::Smooth).toInt());
}

void Settings::setDeviceSmooth(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Device::Smooth, getValidDeviceSmooth(value));
	this->deviceSmoothChanged(value);
}

int Settings::getDeviceColorDepth() const
{
    return getValidDeviceColorDepth(value(Profile::Key::Device::ColorDepth).toInt());
}

void Settings::setDeviceColorDepth(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Device::ColorDepth, getValidDeviceColorDepth(value));
	this->deviceColorDepthChanged(value);
}

double Settings::getDeviceGamma() const
{
    return getValidDeviceGamma(value(Profile::Key::Device::Gamma).toDouble());
}

void Settings::setDeviceGamma(double gamma)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::Device::Gamma, getValidDeviceGamma(gamma));
	this->deviceGammaChanged(gamma);
}

Grab::GrabberType Settings::getGrabberType()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	const QString strGrabber = m_currentProfile.value(Profile::Key::Grab::Grabber).toString();

#ifdef QT_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::Qt)
        return Grab::GrabberTypeQt;
    if (strGrabber == Profile::Value::GrabberType::QtEachWidget)
        return Grab::GrabberTypeQtEachWidget;
#endif

#ifdef WINAPI_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::WinAPI)
        return Grab::GrabberTypeWinAPI;
#endif
#ifdef WINAPI_EACH_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::WinAPIEachWidget)
        return Grab::GrabberTypeWinAPIEachWidget;
#endif

#ifdef D3D9_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::D3D9)
        return Grab::GrabberTypeD3D9;
#endif

#ifdef X11_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::X11)
        return Grab::GrabberTypeX11;
#endif

#ifdef MAC_OS_CG_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::MacCoreGraphics)
        return Grab::GrabberTypeMacCoreGraphics;
#endif

    qWarning() << Q_FUNC_INFO << Profile::Key::Grab::Grabber << "contains invalid value:" << strGrabber << ", reset it to default:" << Profile::Grab::GrabberDefaultString;
    setGrabberType(Profile::Grab::GrabberDefault);

    return Profile::Grab::GrabberDefault;
}

void Settings::setGrabberType(Grab::GrabberType grabberType)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << grabberType;

    QString strGrabber;
    switch (grabberType)
    {
    case Grab::GrabberTypeQt:
        strGrabber = Profile::Value::GrabberType::Qt;
        break;

    case Grab::GrabberTypeQtEachWidget:
        strGrabber = Profile::Value::GrabberType::QtEachWidget;
        break;

#ifdef WINAPI_GRAB_SUPPORT
    case Grab::GrabberTypeWinAPI:
        strGrabber = Profile::Value::GrabberType::WinAPI;
        break;
    case Grab::GrabberTypeWinAPIEachWidget:
        strGrabber = Profile::Value::GrabberType::WinAPIEachWidget;
        break;
#endif

#ifdef D3D9_GRAB_SUPPORT
    case Grab::GrabberTypeD3D9:
        strGrabber = Profile::Value::GrabberType::D3D9;
        break;
#endif

#ifdef X11_GRAB_SUPPORT
    case Grab::GrabberTypeX11:
        strGrabber = Profile::Value::GrabberType::X11;
        break;
#endif

#ifdef MAC_OS_CG_GRAB_SUPPORT
    case Grab::GrabberTypeMacCoreGraphics:
        strGrabber = Profile::Value::GrabberType::MacCoreGraphics;
        break;
#endif

    default:
        qWarning() << Q_FUNC_INFO << "Switch on grabberType =" << grabberType << "failed. Reset to default value.";
        strGrabber = Profile::Grab::GrabberDefaultString;
    }
	m_currentProfile.setValue(Profile::Key::Grab::Grabber, strGrabber);
	this->grabberTypeChanged(grabberType);
}

#ifdef D3D10_GRAB_SUPPORT
bool Settings::isDx1011GrabberEnabled() const {
	return m_currentProfile.value(Profile::Key::Grab::IsDx1011GrabberEnabled).toBool();
}

void Settings::setDx1011GrabberEnabled(bool isEnabled) {
	m_currentProfile.setValue(Profile::Key::Grab::IsDx1011GrabberEnabled, isEnabled);
	this->dx1011GrabberEnabledChanged(isEnabled);
}
#endif

Lightpack::Mode Settings::getLightpackMode()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	const QString strMode = m_currentProfile.value(Profile::Key::LightpackMode).toString();

    if (strMode == Profile::Value::LightpackMode::Ambilight)
    {
        return Lightpack::AmbilightMode;
    }
    else if (strMode == Profile::Value::LightpackMode::MoodLamp)
    {
        return Lightpack::MoodLampMode;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Read fail. Reset to default value = " << Lightpack::Default;

        setLightpackMode(Lightpack::Default);
        return Lightpack::Default;
    }
}

void Settings::setLightpackMode(Lightpack::Mode mode)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << mode;

    if (mode == Lightpack::AmbilightMode)
    {
		m_currentProfile.setValue(Profile::Key::LightpackMode, Profile::Value::LightpackMode::Ambilight);
		this->lightpackModeChanged(mode);
    }
    else if (mode == Lightpack::MoodLampMode)
    {
		m_currentProfile.setValue(Profile::Key::LightpackMode, Profile::Value::LightpackMode::MoodLamp);
		this->lightpackModeChanged(mode);
    }
    else
    {
        qCritical() << Q_FUNC_INFO << "Invalid value =" << mode;
    }
}

bool Settings::isMoodLampLiquidMode() const
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	return m_currentProfile.value(Profile::Key::MoodLamp::IsLiquidMode).toBool();
}

void Settings::setMoodLampLiquidMode(bool value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::MoodLamp::IsLiquidMode, value );
	this->moodLampLiquidModeChanged(value);
}

QColor Settings::getMoodLampColor() const
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    return QColor(value(Profile::Key::MoodLamp::Color).toString());
}

void Settings::setMoodLampColor(QColor value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value.name();
	m_currentProfile.setValue(Profile::Key::MoodLamp::Color, value.name() );
	this->moodLampColorChanged(value);
}

int Settings::getMoodLampSpeed() const
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    return getValidMoodLampSpeed(value(Profile::Key::MoodLamp::Speed).toInt());
}

void Settings::setMoodLampSpeed(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(Profile::Key::MoodLamp::Speed, getValidMoodLampSpeed(value));
	this->moodLampSpeedChanged(value);
}

QList<WBAdjustment> Settings::getLedCoefs() const
{
    QList<WBAdjustment> result;
    const size_t numOfLeds = getNumberOfLeds(getConnectedDevice());

    for(size_t led = 0; led < numOfLeds; ++led)
        result.append(getLedAdjustment(led));

    return result;
}

double Settings::getLedCoefRed(int ledIndex)
{
    return getValidLedCoef(ledIndex, Profile::Key::Led::CoefRed);
}

double Settings::getLedCoefGreen(int ledIndex)
{
    return getValidLedCoef(ledIndex, Profile::Key::Led::CoefGreen);
}

double Settings::getLedCoefBlue(int ledIndex)
{
    return getValidLedCoef(ledIndex, Profile::Key::Led::CoefBlue);
}

void Settings::setLedCoefRed(int ledIndex, double value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    setValidLedCoef(ledIndex, Profile::Key::Led::CoefRed, value);
	this->ledCoefRedChanged(ledIndex, value);
}

void Settings::setLedCoefGreen(int ledIndex, double value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    setValidLedCoef(ledIndex, Profile::Key::Led::CoefGreen, value);
	this->ledCoefGreenChanged(ledIndex, value);
}

void Settings::setLedCoefBlue(int ledIndex, double value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    setValidLedCoef(ledIndex, Profile::Key::Led::CoefBlue, value);
	this->ledCoefBlueChanged(ledIndex, value);
}

QSize Settings::getLedSize(int ledIndex) const
{
	return m_currentProfile.value(
			Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Size
		).toSize();
}

void Settings::setLedSize(int ledIndex, QSize size)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(
			Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Size,
			size);
	this->ledSizeChanged(ledIndex, size);
}

QPoint Settings::getLedPosition(int ledIndex) const
{
	return m_currentProfile.value(
			Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Position
		).toPoint();
}

void Settings::setLedPosition(int ledIndex, QPoint position)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(
			Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Position,
			position);
	this->ledPositionChanged(ledIndex, position);
}

bool Settings::isLedEnabled(int ledIndex) const
{
	return m_currentProfile.valueOrDefault(
			Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::IsEnabled,
			Profile::Led::IsEnabledDefault).toBool();
}

void Settings::setLedEnabled(int ledIndex, bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	m_currentProfile.setValue(
			Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::IsEnabled,
			isEnabled);
	this->ledEnabledChanged(ledIndex, isEnabled);
}

// static
int Settings::getValidDeviceRefreshDelay(int value)
{
	return clamp_value(value, Profile::Device::RefreshDelayMin, Profile::Device::RefreshDelayMax);
}

// static
int Settings::getValidDeviceBrightness(int value)
{
	return clamp_value(value, Profile::Device::BrightnessMin, Profile::Device::BrightnessMax);
}

// static
int Settings::getValidDeviceSmooth(int value)
{
	return clamp_value(value, Profile::Device::SmoothMin, Profile::Device::SmoothMax);
}

// static
int Settings::getValidDeviceColorDepth(int value)
{
	return clamp_value(value, Profile::Device::ColorDepthMin, Profile::Device::ColorDepthMin);
}

// static
double Settings::getValidDeviceGamma(double value)
{
	return clamp_value(value, Profile::Device::GammaMin, Profile::Device::GammaMin);
}

// static
int Settings::getValidGrabSlowdown(int value)
{
	return clamp_value(value, Profile::Grab::SlowdownMin, Profile::Grab::SlowdownMax);
}

// static
int Settings::getValidMoodLampSpeed(int value)
{
	return clamp_value(value, Profile::MoodLamp::SpeedMin, Profile::MoodLamp::SpeedMax);
}

// static
int Settings::getValidLuminosityThreshold(int value)
{
	return clamp_value(value, Profile::Grab::MinimumLevelOfSensitivityMin, Profile::Grab::MinimumLevelOfSensitivityMin);
}

namespace {
inline bool validateLedCoef(double coef, bool conversionResult, QString& errorDescription) {
	if (!conversionResult) {
		errorDescription = "Error: Convert to double.";
		return false;
	} else if (between(coef, Profile::Led::CoefMin, Profile::Led::CoefMax)){
		errorDescription = "Error: outside the valid values (coef < " +
				QString::number(Profile::Led::CoefMin) + " || coef > " + QString::number(Profile::Led::CoefMax) + ").";
		return false;
	}

	return true;
}

inline double checkedLedCoef(const QString& prefix, const QString keyCoef, double coef, bool conversionResult) {
	QString error;
	if (validateLedCoef(coef, conversionResult, error))
		return coef;

	coef = Profile::Led::CoefDefault;
	qWarning() << Q_FUNC_INFO << "Checking bad value"
			   << "[" + prefix + "]"
			   << keyCoef
			   << error
			   << ". Set it to default value" << keyCoef << "=" << coef;
	return coef;
}
}

void Settings::setValidLedCoef(int ledIndex, const QString & keyCoef, double coef)
{
	const QString prefix(Profile::Key::Led::Prefix + QString::number(ledIndex + 1));
	coef = checkedLedCoef(prefix, keyCoef, coef, true);
	m_currentProfile.setValue(prefix + "/" + keyCoef, coef);
}

double Settings::getValidLedCoef(int ledIndex, const QString & keyCoef)
{
    bool ok = false;
	const QString prefix(Profile::Key::Led::Prefix + QString::number(ledIndex + 1));
	double coef = Settings::value(prefix + "/" + keyCoef).toDouble(&ok);
	double checkedCoef = checkedLedCoef(prefix, keyCoef, coef, ok);
	if (coef == checkedCoef)
		return coef;

	m_currentProfile.setValue(prefix + "/" + keyCoef, checkedCoef);
	return checkedCoef;
}

QString Settings::getProfilesPath() const {
   return m_applicationDirPath + "Profiles/";
}

uint Settings::getLastReadUpdateId() const {
   return m_mainProfile.value(Main::Key::LastReadUpdateId).toUInt();
}

void Settings::setLastReadUpdateId(const uint updateId) {
	m_mainProfile.setValue(Main::Key::LastReadUpdateId, updateId);
}

void Settings::setValueMain(const QString & key, const QVariant & value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << value;

	m_mainProfile.setValue(key, value);
}

QVariant Settings::valueMain(const QString & key) const
{
	QVariant value = m_mainProfile.value(key);
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

void Settings::setValue(const QString & key, const QVariant & value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;

	m_currentProfile.setValue(key, value);
}

QVariant Settings::value(const QString & key) const
{
	QVariant value = m_currentProfile.value(key);
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

QVariant Settings::pluginValue(const QString & pluginId, const QString & key) const {
	const QString pluginKey = pluginId + "/" + key;
	return m_mainProfile.value(pluginKey);
}

void Settings::setPluginValue(const QString & pluginId, const QString & key, const QVariant& value) {
	const QString pluginKey = pluginId + "/" + key;
	return m_mainProfile.setValue(pluginKey, value);
}

void Settings::initDevicesMap()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	m_devicesTypeToNameMap[SupportedDevices::DefaultDeviceType]  = Main::Value::ConnectedDevice::AdalightDevice;
    m_devicesTypeToNameMap[SupportedDevices::DeviceTypeAdalight]  = Main::Value::ConnectedDevice::AdalightDevice;
    m_devicesTypeToNameMap[SupportedDevices::DeviceTypeArdulight] = Main::Value::ConnectedDevice::ArdulightDevice;
    m_devicesTypeToNameMap[SupportedDevices::DeviceTypeLightpack] = Main::Value::ConnectedDevice::LightpackDevice;
    m_devicesTypeToNameMap[SupportedDevices::DeviceTypeVirtual]   = Main::Value::ConnectedDevice::VirtualDevice;

    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::DeviceTypeAdalight]  = Main::Key::Adalight::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::DeviceTypeArdulight] = Main::Key::Ardulight::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::DeviceTypeLightpack] = Main::Key::Lightpack::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::DeviceTypeVirtual]   = Main::Key::Virtual::NumberOfLeds;

#ifdef ALIEN_FX_SUPPORTED
    m_devicesTypeToNameMap[SupportedDevices::DeviceTypeAlienFx]   = Main::Value::ConnectedDevice::AlienFxDevice;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::DeviceTypeAlienFx]   = Main::Key::AlienFx::NumberOfLeds;
#endif

	if (!m_devicesTypeToNameMap.contains(SupportedDevices::DefaultDeviceType)) {
		m_devicesTypeToNameMap[SupportedDevices::DefaultDeviceType] = Main::ConnectedDeviceDefault;
	}
}

/*
 * --------------------------- Migration --------------------------------
 */


void Settings::migrateSettings()
{
	static const BaseVersion kVersion_1_0(1, 0);
	static const BaseVersion kVersion_2_0(2, 0);
	static const BaseVersion kVersion_3_0(3, 0);
	static const BaseVersion kVersion_4_0(4, 0);

	const BaseVersion configVersion(valueMain(Main::Key::MainConfigVersion).toString());
	if (configVersion == kVersion_1_0) {

		if (getConnectedDevice() == SupportedDevices::DeviceTypeLightpack) {

			const int remap[] = {3, 4, 2, 1, 0, 5, 6, 7, 8, 9};

			const size_t ledCount = getNumberOfLeds(SupportedDevices::DeviceTypeLightpack);
            QMap<int, LedInfo> ledInfoMap;
            for(size_t i = 0; i < ledCount; i++){
                LedInfo ledInfo;
                ledInfo.isEnabled = isLedEnabled(i);
                ledInfo.position = getLedPosition(i);
                ledInfo.size = getLedSize(i);
                ledInfo.wbRed = getLedCoefRed(i);
                ledInfo.wbGreen = getLedCoefGreen(i);
                ledInfo.wbBlue = getLedCoefBlue(i);
                ledInfoMap.insert(remap[i], ledInfo);
            }

			const QList<LedInfo> remappedLeds = ledInfoMap.values();
            for (int i = 0; i < remappedLeds.size(); ++i) {
                Settings::setLedEnabled(i, remappedLeds[i].isEnabled);
                Settings::setLedPosition(i, remappedLeds[i].position);
                Settings::setLedSize(i, remappedLeds[i].size);
                Settings::setLedCoefRed(i, remappedLeds[i].wbRed);
                Settings::setLedCoefGreen(i, remappedLeds[i].wbGreen);
                Settings::setLedCoefBlue(i, remappedLeds[i].wbBlue);
            }
        }
		setValueMain(Main::Key::MainConfigVersion, kVersion_2_0.toString());
    }
	if (configVersion == kVersion_2_0) {
        // Disable ApiAuth by default
        setApiKey("");

        // Remove obsolete keys
		const QString authEnabledKey = "API/IsAuthEnabled";
		m_mainProfile.remove(authEnabledKey);

		setValueMain(Main::Key::MainConfigVersion, kVersion_3_0.toString());
    }
	if (configVersion == kVersion_3_0) {
#ifdef WINAPI_GRAB_SUPPORT
        Settings::setGrabberType(Grab::GrabberTypeWinAPI);
#endif

#ifdef X11_GRAB_SUPPORT
        Settings::setGrabberType(Grab::GrabberTypeX11);
#endif

#ifdef MAC_OS_CG_GRAB_SUPPORT
        Settings::setGrabberType(Grab::GrabberTypeMacCoreGraphics);
#endif

		setValueMain(Main::Key::MainConfigVersion, kVersion_4_0.toString());
    }
}
} /*SettingsScope*/
