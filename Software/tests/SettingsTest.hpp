#ifndef SETTINGSTEST_HPP
#define SETTINGSTEST_HPP

#include <QTest>

class SettingsTest : public QObject
{
    Q_OBJECT
public:
    SettingsTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testCase_initMockSettings();
    void testCase_verifyMainSettings();
    void testCase_migrateMainSettingsFrom1_0();
    void testCase_migrateMainSettingsFrom2_0();
    void testCase_migrateMainSettingsFrom3_0();
    void testCase_resetDefault();
};

#endif // SETTINGSTEST_HPP
