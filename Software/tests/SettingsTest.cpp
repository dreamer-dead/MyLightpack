#include <QMap>

#include "SettingsSourceMockup.hpp"
#include "SettingsDefaults.hpp"
#include "SettingsTest.hpp"
#include "Settings.hpp"
#include "BaseVersion.hpp"

using namespace SettingsScope;

namespace {
static SettingsSource* SettingsSourceQMapFabricFunc(const QString&) {
    return new SettingsSourceMockup();
}
} // namespace

SettingsTest::SettingsTest() : QObject() {
}

void SettingsTest::init() {
    g_debugLevel = Debug::MidLevel;
    ConfigurationProfile::setSourceFabric(&SettingsSourceQMapFabricFunc);
    QVERIFY(!Settings::instance());
}

void SettingsTest::cleanup() {
    QVERIFY(Settings::instance());
    Settings::Shutdown();
    QVERIFY(!Settings::instance());
    ConfigurationProfile::setSourceFabric(NULL);
}

void SettingsTest::initTestCase() {
    QVERIFY(!Settings::instance());
}

void SettingsTest::cleanupTestCase() {
    QVERIFY(!Settings::instance());
}

void SettingsTest::testCase_initMockSettings() {
    g_debugLevel = Debug::MidLevel;

    // There is no settings files.
    QVERIFY(!Settings::Initialize("./", Settings::Overrides()));
    QVERIFY(Settings::instance());
}

void SettingsTest::testCase_verifyMainSettings() {
    g_debugLevel = Debug::MidLevel;

    // Set invalid device.
    Settings::TestingOverrides overrides;
    overrides.setConnectedDeviceForTests(SupportedDevices::DeviceTypesCount);

    // There is no settings files.
    QVERIFY(!Settings::Initialize("./", overrides));
    QVERIFY(Settings::instance()->getConnectedDevice() != SupportedDevices::DeviceTypesCount);
    QCOMPARE(Settings::instance()->getConnectedDevice(), SupportedDevices::DefaultDeviceType);
}

void SettingsTest::testCase_migrateMainSettingsFrom1_0() {
    g_debugLevel = Debug::MidLevel;

    // Set minimal config version.
    Settings::TestingOverrides overrides;
    overrides.setConfigVersionForTests(BaseVersion(1, 0));
    QVERIFY(!Settings::Initialize("./", overrides));
    QCOMPARE(Settings::instance()->getVersion(), BaseVersion(4, 0));
}

void SettingsTest::testCase_migrateMainSettingsFrom2_0() {
    g_debugLevel = Debug::MidLevel;

    // Set minimal config version.
    Settings::TestingOverrides overrides;
    overrides.setConfigVersionForTests(BaseVersion(2, 0));
    QVERIFY(!Settings::Initialize("./", overrides));
    QCOMPARE(Settings::instance()->getVersion(), BaseVersion(4, 0));
}

void SettingsTest::testCase_migrateMainSettingsFrom3_0() {
    g_debugLevel = Debug::MidLevel;

    // Set minimal config version.
    Settings::TestingOverrides overrides;
    overrides.setConfigVersionForTests(BaseVersion(3, 0));
    QVERIFY(!Settings::Initialize("./", overrides));
    QCOMPARE(Settings::instance()->getVersion(), BaseVersion(4, 0));
}

void SettingsTest::testCase_resetDefault() {
    g_debugLevel = Debug::MidLevel;
    QVERIFY(!Settings::Initialize("./", Settings::Overrides()));
    QVERIFY(Settings::instance());

    // Set some non-default settings.
    // TODO: check all settings that listed in resetDefaults()
    Settings::instance()->setMoodLampLiquidMode(!Profile::MoodLamp::IsLiquidMode);
    Settings::instance()->resetDefaults();

    // Check that now settings filled with default values.
    QCOMPARE(Settings::instance()->isMoodLampLiquidMode(), Profile::MoodLamp::IsLiquidMode);
}
