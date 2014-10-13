#include <QMap>

#include "BaseVersion.hpp"
#include "Settings.hpp"
#include "SettingsDefaults.hpp"
#include "common/DebugOut.hpp"
#include "gtest/gtest.h"
#include "mocks/SettingsSourceMockup.hpp"

using namespace SettingsScope;

namespace {
static SettingsSource* SettingsSourceQMapFabricFunc(const QString&) {
    return new SettingsSourceMockup();
}

class SettingsTest : public ::testing::Test {
public:
    SettingsTest() {}

    virtual void SetUp() {
        g_debugLevel = Debug::ZeroLevel;
        s_errorCount = 0;
        // Suppress any Qt messages.
        m_oldHandler = qInstallMessageHandler(&SettingsTest::messageHandler);
        ConfigurationProfile::setSourceFabric(&SettingsSourceQMapFabricFunc);
        EXPECT_FALSE(Settings::instance());
    }

    virtual void TearDown() {
        EXPECT_EQ(0u, s_errorCount);
        EXPECT_TRUE(Settings::instance());
        Settings::Shutdown();
        EXPECT_FALSE(Settings::instance());
        ConfigurationProfile::setSourceFabric(NULL);
        // Restore old handler.
        qInstallMessageHandler(m_oldHandler);
    }

private:
    static size_t s_errorCount;
    static void messageHandler(QtMsgType type, const QMessageLogContext&, const QString&) {
        if (type == QtCriticalMsg || type == QtFatalMsg) {
            ++s_errorCount;
        }
    }

    QtMessageHandler m_oldHandler;
};

//static
size_t SettingsTest::s_errorCount = 0;
} // namespace

TEST_F(SettingsTest, initMockSettings) {
    // There is no settings files.
    EXPECT_TRUE(Settings::Initialize("./", Settings::Overrides()));
    EXPECT_TRUE(Settings::instance());
}

TEST_F(SettingsTest, verifyMainSettings) {
    // Set invalid device.
    Settings::TestingOverrides overrides;
    overrides.setConnectedDeviceForTests(SupportedDevices::DeviceTypesCount);

    // There is no settings files.
    EXPECT_TRUE(Settings::Initialize("./", overrides));
    EXPECT_NE(SupportedDevices::DeviceTypesCount, Settings::instance()->getConnectedDevice());
    EXPECT_EQ(SupportedDevices::DefaultDeviceType, Settings::instance()->getConnectedDevice());
}

TEST_F(SettingsTest, migrateMainSettingsFrom1_0) {
    // Set minimal config version.
    Settings::TestingOverrides overrides;
    overrides.setConfigVersionForTests(BaseVersion(1, 0));
    EXPECT_TRUE(Settings::Initialize("./", overrides));
    EXPECT_EQ(BaseVersion(4, 0), Settings::instance()->getVersion());
}

TEST_F(SettingsTest, migrateMainSettingsFrom2_0) {
    // Set minimal config version.
    Settings::TestingOverrides overrides;
    overrides.setConfigVersionForTests(BaseVersion(2, 0));
    EXPECT_TRUE(Settings::Initialize("./", overrides));
    EXPECT_EQ(BaseVersion(4, 0), Settings::instance()->getVersion());
}

TEST_F(SettingsTest, migrateMainSettingsFrom3_0) {
    // Set minimal config version.
    Settings::TestingOverrides overrides;
    overrides.setConfigVersionForTests(BaseVersion(3, 0));
    EXPECT_TRUE(Settings::Initialize("./", overrides));
    EXPECT_EQ(BaseVersion(4, 0), Settings::instance()->getVersion());
}

TEST_F(SettingsTest, resetDefault) {
    EXPECT_TRUE(Settings::Initialize("./", Settings::Overrides()));
    EXPECT_TRUE(Settings::instance());

    // Set some non-default settings.
    // TODO: check all settings that listed in resetDefaults()
    Settings::instance()->setMoodLampLiquidMode(!Profile::MoodLamp::IsLiquidMode);
    Settings::instance()->resetDefaults();

    // Check that now settings filled with default values.
    EXPECT_EQ(Profile::MoodLamp::IsLiquidMode, Settings::instance()->isMoodLampLiquidMode());
}
