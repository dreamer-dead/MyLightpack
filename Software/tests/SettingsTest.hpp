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

	void init();
	void cleanup();

	void testCase_initMockSettings();
};

#endif // SETTINGSTEST_HPP
