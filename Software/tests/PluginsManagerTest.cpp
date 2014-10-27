#include <QSignalSpy>
#include <QTimer>
#include <QProcess>

#include "Settings.hpp"
#include "Plugin.hpp"
#include "PluginsManager.hpp"
#include "common/PrintHelpers.hpp"
#include "gtest/gtest.h"
#include "mocks/SettingsSourceMockup.hpp"

using namespace SettingsScope;

namespace {
static SettingsSource* SettingsSourceQMapFabricFunc(const QString&) {
    return new SettingsSourceMockup();
}
}

TEST(PluginsManagerTest, CreateManager) {
    const QString pluginsDir(SRCDIR "data/plugins/");
    ASSERT_TRUE(QDir(pluginsDir).exists()) << QDir::currentPath();
    PluginsManager manager(pluginsDir);

    EXPECT_FALSE(manager.getPlugin("info"));
    EXPECT_FALSE(manager.getPlugin("simple"));
    EXPECT_FALSE(manager.getPlugin("kill"));
    QList<Plugin*> plugins = manager.getPluginList();
    EXPECT_TRUE(plugins.isEmpty());
}

TEST(PluginsManagerTest, LoadPlugins) {
    const QString pluginsDir(SRCDIR "data/plugins/");
    ASSERT_TRUE(QDir(pluginsDir).exists()) << QDir::currentPath();
    g_debugLevel = Debug::ZeroLevel;
    ConfigurationProfile::setSourceFabric(&SettingsSourceQMapFabricFunc);
    EXPECT_TRUE(Settings::Initialize("./", Settings::Overrides()));
    EXPECT_TRUE(Settings::instance());

    PluginsManager manager(pluginsDir);

    QSignalSpy managerSpy(&manager, SIGNAL(updatePlugin(QList<Plugin *>)));
    QTimer::singleShot(0, &manager, SLOT(reloadPlugins()));
    EXPECT_TRUE(managerSpy.wait(500));
    QList<Plugin*> plugins = manager.getPluginList();
    EXPECT_FALSE(plugins.isEmpty());
    EXPECT_EQ(3, plugins.size());
    EXPECT_TRUE(manager.getPlugin("info"));
    EXPECT_TRUE(manager.getPlugin("simple"));
    EXPECT_TRUE(manager.getPlugin("kill"));

    Settings::Shutdown();
    EXPECT_FALSE(Settings::instance());
    ConfigurationProfile::setSourceFabric(NULL);
}

TEST(PluginsManagerTest, LoadAndStartPlugins) {
    const QString pluginsDir(SRCDIR "data/plugins/");
    ASSERT_TRUE(QDir(pluginsDir).exists()) << QDir::currentPath();
    g_debugLevel = Debug::ZeroLevel;
    ConfigurationProfile::setSourceFabric(&SettingsSourceQMapFabricFunc);
    EXPECT_TRUE(Settings::Initialize("./", Settings::Overrides()));
    EXPECT_TRUE(Settings::instance());

    PluginsManager manager(pluginsDir);

    QSignalSpy managerSpy(&manager, SIGNAL(updatePlugin(QList<Plugin *>)));
    QTimer::singleShot(0, &manager, SLOT(reloadPlugins()));
    EXPECT_TRUE(managerSpy.wait(500));

    Plugin* longRunningPlugin = manager.getPlugin("kill");
    ASSERT_TRUE(longRunningPlugin);
    EXPECT_FALSE(longRunningPlugin->isEnabled());
    EXPECT_EQ(QProcess::NotRunning, longRunningPlugin->state());
    longRunningPlugin->setEnabled(true);
    EXPECT_TRUE(longRunningPlugin->isEnabled());

    QTimer::singleShot(0, &manager, SLOT(reloadPlugins()));
    EXPECT_TRUE(managerSpy.wait(500));
    longRunningPlugin = manager.getPlugin("kill");
    ASSERT_TRUE(longRunningPlugin);
    QSignalSpy processSpy(longRunningPlugin, SIGNAL(stateChanged(QProcess::ProcessState)));
    EXPECT_TRUE(longRunningPlugin->isEnabled());
    processSpy.wait(500);
    processSpy.wait(500);
    EXPECT_EQ(QProcess::Running, longRunningPlugin->state());

    QTimer::singleShot(0, &manager, SLOT(stopPlugins()));
    EXPECT_TRUE(processSpy.wait(100));
    EXPECT_EQ(QProcess::NotRunning, longRunningPlugin->state());

    Settings::Shutdown();
    ConfigurationProfile::setSourceFabric(NULL);
}

