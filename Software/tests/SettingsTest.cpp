#include <QMap>
#include <QTest>

#include "SettingsTest.hpp"
#include "Settings.hpp"

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

void SettingsTest::init() {
}

void SettingsTest::cleanup() {
}

void SettingsTest::testCase_initMockSettings() {
	ConfigurationProfile::setSourceFabric(&SettingsSourceQMapFabricFunc);

	// There is no settings files.
	QVERIFY(!Settings::Initialize("./", Settings::Overrides()));
}
