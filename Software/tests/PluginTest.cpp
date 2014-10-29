#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTimer>

#include "Plugin.hpp"
#include "common/PrintHelpers.hpp"
#include "gtest/gtest.h"
#include "mocks/ProcessWaiter.hpp"

TEST(PluginTest, ReadInfo) {
    const QString iniName(SRCDIR "data/plugins/info/info.ini");
    ASSERT_TRUE(QFile(iniName).exists()) << QDir::currentPath();
    QSettings settings(iniName, QSettings::IniFormat);
    PluginInfo info;
    Plugin::readPluginInfo(settings, &info);
    EXPECT_EQ(QString("TestInfoPlugin"), info.name) << info.name;
    EXPECT_EQ(QString("uname"), info.exec);
    EXPECT_EQ(QString("GUID"), info.guid);
    EXPECT_EQ(QString("Dreamer"), info.author);
    EXPECT_EQ(QString("Simple test plugin info."), info.description);
    EXPECT_EQ(QString("0.0.0.1"), info.version);
    EXPECT_EQ(QString("res/icon.png"), info.icon);
}

TEST(PluginTest, LoadSimplePlugin) {
    const QDir pluginDir(SRCDIR "data/plugins/simple/");
    const QString iniName(pluginDir.absoluteFilePath("simple.ini"));
    ASSERT_TRUE(QFile(iniName).exists()) << QDir::currentPath();

    Plugin plugin("simple", pluginDir.absolutePath());
    EXPECT_EQ(QString("TestSimplePlugin"), plugin.Name()) << plugin.Name();
    EXPECT_EQ(QString("GUID"), plugin.Guid());
    EXPECT_EQ(QString("Dreamer"), plugin.Author());
    EXPECT_EQ(QString("Simple test plugin info."), plugin.Description());
    EXPECT_EQ(QString("0.0.0.1"), plugin.Version());

    EXPECT_EQ(QProcess::NotRunning, plugin.state());
    ProcessWaiter processSpy(&plugin, SIGNAL(stateChanged(QProcess::ProcessState)));
    EXPECT_TRUE(processSpy.isValid());
    plugin.Start();
    EXPECT_TRUE(processSpy.wait(QProcess::Starting, 1000));
    EXPECT_TRUE(processSpy.wait(QProcess::Running, 1000));
    EXPECT_TRUE(processSpy.wait(QProcess::NotRunning, 3000));
}

TEST(PluginTest, KillLongRunningPlugin) {
    const QDir pluginDir(SRCDIR "data/plugins/kill/");
    const QString iniName(pluginDir.absoluteFilePath("kill.ini"));
    ASSERT_TRUE(QFile(iniName).exists()) << QDir::currentPath();

    Plugin plugin("kill", pluginDir.absolutePath());
    ProcessWaiter processSpy(&plugin, SIGNAL(stateChanged(QProcess::ProcessState)));
    EXPECT_TRUE(processSpy.isValid());

    EXPECT_EQ(QProcess::NotRunning, plugin.state());
    QTimer::singleShot(0, &plugin, SLOT(Start()));
    EXPECT_TRUE(processSpy.wait(QProcess::Starting, 500));
    EXPECT_TRUE(processSpy.wait(QProcess::Running, 1500));
    // Make sure that plugin is still runnning.
    EXPECT_FALSE(processSpy.wait(QProcess::NotRunning, 1000));

    QTimer::singleShot(0, &plugin, SLOT(Stop()));
    EXPECT_TRUE(processSpy.wait(QProcess::NotRunning, 500));
}

