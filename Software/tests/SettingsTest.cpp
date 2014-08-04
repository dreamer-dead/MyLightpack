#include <QMap>
#include <QTest>

#include "SettingsTest.hpp"
#include "Settings.hpp"
#include "BaseVersion.hpp"

using namespace SettingsScope;

namespace {
class SettingsSourceQMap : public SettingsSource {
public:
	virtual QVariant value(const QString & key) const {
		return m_settingsMap.value(key);
	}

	virtual void setValue(const QString & key, const QVariant & value) {
		m_settingsMap.insert(key, value);
	}

	virtual bool contains(const QString& key) const {
		return m_settingsMap.contains(key);
	}

	virtual void remove(const QString& key) {
		m_settingsMap.remove(key);
	}

	virtual void sync() {}

private:
	QMap<QString, QVariant> m_settingsMap;
};

static SettingsSource* SettingsSourceQMapFabricFunc(const QString&) {
	return new SettingsSourceQMap();
}
}

SettingsTest::SettingsTest() : QObject() {
}

void SettingsTest::initTestCase() {
}

void SettingsTest::cleanupTestCase() {
}

void SettingsTest::init() {
    ConfigurationProfile::setSourceFabric(&SettingsSourceQMapFabricFunc);
}

void SettingsTest::cleanup() {
    Settings::Shutdown();
    ConfigurationProfile::setSourceFabric(NULL);
}

void SettingsTest::testCase_initMockSettings() {
    g_debugLevel = Debug::MidLevel;

	// There is no settings files.
	QVERIFY(!Settings::Initialize("./", Settings::Overrides()));
    QVERIFY(Settings::instance());
}

void SettingsTest::testCase_verifyMainSettings() {
    g_debugLevel = Debug::MidLevel;

    // There is no settings files.
    // Set invalid device.
	const Settings::Overrides& overrides = Settings::TestingOverrides()
			.setConnectedDeviceForTests(SupportedDevices::DeviceTypesCount);
    QVERIFY(!Settings::Initialize("./", overrides));
    QVERIFY(Settings::instance()->getConnectedDevice() != SupportedDevices::DeviceTypesCount);
    QCOMPARE(Settings::instance()->getConnectedDevice(), SupportedDevices::DefaultDeviceType);
}

void SettingsTest::testCase_migrateMainSettingsFrom1_0() {
    g_debugLevel = Debug::MidLevel;

	// Set minimal config version.
	const Settings::Overrides& overrides = Settings::TestingOverrides()
			.setConfigVersionForTests(BaseVersion(1, 0));
    QVERIFY(!Settings::Initialize("./", overrides));
	QCOMPARE(Settings::instance()->getVersion(), BaseVersion(4, 0));
}
