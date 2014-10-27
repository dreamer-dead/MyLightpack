#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>

#include "Plugin.hpp"
#include "common/PrintHelpers.hpp"
#include "gtest/gtest.h"

TEST(PluginTest, ReadInfo) {
    const QString iniName(SRCDIR "data/plugins/info/plugin.ini");
    ASSERT_TRUE(QFile(iniName).exists()) << QDir::currentPath();
    QSettings settings(iniName, QSettings::IniFormat);
    PluginInfo info;
    Plugin::readPluginInfo(settings, &info);
    EXPECT_EQ(QString("TestPlugin"), info.name) << info.name;
    EXPECT_EQ(QString("uname"), info.exec);
    EXPECT_EQ(QString("GUID"), info.guid);
    EXPECT_EQ(QString("Dreamer"), info.author);
    EXPECT_EQ(QString("Simple test plugin info."), info.description);
    EXPECT_EQ(QString("0.0.0.1"), info.version);
    EXPECT_EQ(QString("res/icon.png"), info.icon);
}

TEST(PluginTest, LoadSimplePlugin) {
    const QDir pluginDir(SRCDIR "data/plugins/simple/");
    const QString iniName(pluginDir.absoluteFilePath("plugin.ini"));
    ASSERT_TRUE(QFile(iniName).exists()) << QDir::currentPath();

    Plugin plugin("plugin", pluginDir.absolutePath());
    EXPECT_EQ(QString("TestPlugin"), plugin.Name()) << plugin.Name();
    EXPECT_EQ(QString("GUID"), plugin.Guid());
    EXPECT_EQ(QString("Dreamer"), plugin.Author());
    EXPECT_EQ(QString("Simple test plugin info."), plugin.Description());
    EXPECT_EQ(QString("0.0.0.1"), plugin.Version());

    EXPECT_EQ(QProcess::NotRunning, plugin.state());
    QSignalSpy processSpy(&plugin, SIGNAL(stateChanged(QProcess::ProcessState)));
    EXPECT_TRUE(processSpy.isValid());
    plugin.Start();
    EXPECT_EQ(QProcess::Starting, plugin.state());
    EXPECT_TRUE(processSpy.wait(1000));
    processSpy.wait(1000);
    EXPECT_EQ(QProcess::NotRunning, plugin.state());
}

TEST(PluginTest, KillLongRunningPlugin) {
    const QDir pluginDir(SRCDIR "data/plugins/kill/");
    const QString iniName(pluginDir.absoluteFilePath("plugin.ini"));
    ASSERT_TRUE(QFile(iniName).exists()) << QDir::currentPath();

    Plugin plugin("plugin", pluginDir.absolutePath());
    QSignalSpy processSpy(&plugin, SIGNAL(stateChanged(QProcess::ProcessState)));
    EXPECT_TRUE(processSpy.isValid());

    EXPECT_EQ(QProcess::NotRunning, plugin.state());
    plugin.Start();
    EXPECT_EQ(QProcess::Starting, plugin.state());
    EXPECT_TRUE(processSpy.wait(500));
    EXPECT_EQ(QProcess::Running, plugin.state());
    // Make sure that plugin is still runnning.
    EXPECT_FALSE(processSpy.wait(1000));
    EXPECT_EQ(QProcess::Running, plugin.state());
    plugin.Stop();
    EXPECT_TRUE(processSpy.wait(500));
    EXPECT_EQ(QProcess::NotRunning, plugin.state());
}

