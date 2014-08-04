#ifndef SETTINGSTEST_HPP
#define SETTINGSTEST_HPP

#include <QObject>

class SettingsTest : public QObject
{
	Q_OBJECT
public:
	SettingsTest();

private Q_SLOTS:
	void initTestCase();
    void cleanupTestCase();

	void init();
	void cleanup();

	void testCase_initMockSettings();
    void testCase_verifyMainSettings();
    void testCase_migrateMainSettings();
};

#endif // SETTINGSTEST_HPP
