#include "SettingsSourceMockupTest.hpp"
#include "SettingsSourceMockup.hpp"

SettingsSourceMockupTest::SettingsSourceMockupTest(QObject *parent) :
	QObject(parent) {
}

void SettingsSourceMockupTest::testCase_AddValue() {
	SettingsSourceMockup settingsSource;
	const QString key("SomeKey");
	const QVariant value(100500);

	QVERIFY(!settingsSource.contains(key));
	settingsSource.setValue(key, value);
	QVERIFY(settingsSource.contains(key));
	QCOMPARE(settingsSource.value(key), value);
}

void SettingsSourceMockupTest::testCase_RemoveValue() {
	SettingsSourceMockup settingsSource;
	const QString key("SomeKey");
	const QVariant value(100500);

	settingsSource.setValue(key, value);
	QVERIFY(settingsSource.contains(key));
	settingsSource.remove(key);
	QVERIFY(!settingsSource.contains(key));
}
