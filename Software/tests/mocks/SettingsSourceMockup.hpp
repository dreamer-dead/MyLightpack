#ifndef SETTINGSSOURCEMOCKUP_H
#define SETTINGSSOURCEMOCKUP_H

#include <QMap>
#include <QString>
#include <QVariant>
#include "SettingsSource.hpp"

class SettingsSourceMockup : public SettingsScope::SettingsSource
{
public:
	SettingsSourceMockup();

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

#endif // SETTINGSSOURCEMOCKUP_H
