#include "ConfigurationProfile.hpp"

#include <QSettings>

#include "debug.h"

namespace SettingsScope {

namespace {
class QSettingsSource : public SettingsSource
{
public:
    QSettingsSource(const QString& path) : m_settings(path, QSettings::IniFormat)
    {
        m_settings.setIniCodec("UTF-8");
        DEBUG_LOW_LEVEL << "Settings file:" << m_settings.fileName();
    }

    virtual QVariant value(const QString & key) const
    {
        return m_settings.value(key);
    }

    virtual void setValue(const QString & key, const QVariant & value)
    {
        m_settings.setValue(key, value);
    }

    virtual bool contains(const QString& key) const
    {
        return m_settings.contains(key);
    }

    virtual void remove(const QString& key)
    {
        m_settings.remove(key);
    }

    virtual void sync() { m_settings.sync(); }

private:
    QSettings m_settings;
};

struct ConditionalMutexLocker
{
    QMutex& m_mutex;
    const bool m_shouldLock;

    ConditionalMutexLocker(QMutex& mutex, bool lock)
        : m_mutex(mutex), m_shouldLock(lock)
    {
        if (lock)
            m_mutex.lock();
    }

    ~ConditionalMutexLocker() { if (m_shouldLock) m_mutex.unlock(); }
};

static ConfigurationProfile::SettingsSourceFabricFunc g_settingsSourceFabric = NULL;
}

// static
void ConfigurationProfile::setSourceFabric(SettingsSourceFabricFunc fabric)
{
    Q_ASSERT((fabric && !g_settingsSourceFabric) || (!fabric && g_settingsSourceFabric));
    g_settingsSourceFabric = fabric;
}

ConfigurationProfile::ConfigurationProfile()
    : m_isInBatchUpdate(false)
{
}

bool ConfigurationProfile::init(const QString& path, const QString& name)
{
    if (g_settingsSourceFabric)
    {
        m_settings.reset(g_settingsSourceFabric(path));
    }
    else
    {
        m_settings.reset(new QSettingsSource(path));
    }
    m_name = name;
    m_path = path;
    return isInitialized();
}

QVariant ConfigurationProfile::value(const QString & key) const
{
    if (!m_settings)
    {
        qWarning() << Q_FUNC_INFO << "m_settings == NULL";
        return QVariant();
    }

    QVariant value;
    {
        ConditionalMutexLocker locker(m_mutex, !m_isInBatchUpdate);
        value = m_settings->value(key);
    }
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

QVariant ConfigurationProfile::valueOrDefault(const QString & key, const QVariant& defaultValue) const
{
    if (!m_settings)
    {
        qWarning() << Q_FUNC_INFO << "m_settings == NULL";
        return defaultValue;
    }

    QVariant value = defaultValue;
    {
        ConditionalMutexLocker locker(m_mutex, !m_isInBatchUpdate);
        if (m_settings->contains(key))
            value = m_settings->value(key);
    }
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

bool ConfigurationProfile::contains(const QString& key) const
{
    if (!m_settings)
        return false;

    return m_settings->contains(key);
}

void ConfigurationProfile::setValue(const QString & key, const QVariant & value, bool force)
{
    if (!m_settings)
    {
        qWarning() << "ConfigurationProfile wasn't initialized.";
        return;
    }

    ConditionalMutexLocker locker(m_mutex, !m_isInBatchUpdate);
    if (force)
    {
        m_settings->setValue(key, value);
    }
    else
    {
        if (!m_settings->contains(key))
        {
            DEBUG_LOW_LEVEL << "Settings:"<< key << "not found."
                            << "Set it to default value: " << value.toString();

            m_settings->setValue(key, value);
        }
        // else option exists do nothing
    }
}

void ConfigurationProfile::remove(const QString& key)
{
    Q_ASSERT(isInitialized());
    if (!isInitialized())
        return;

    m_settings->remove(key);
}

bool ConfigurationProfile::beginBatchUpdate()
{
    Q_ASSERT(!m_isInBatchUpdate);
    if (m_isInBatchUpdate)
        return false;
    m_mutex.lock();
    m_isInBatchUpdate = true;
    return true;
}

void ConfigurationProfile::endBatchUpdate()
{
    Q_ASSERT(m_isInBatchUpdate);
    m_mutex.unlock();
    m_settings->sync();
    m_isInBatchUpdate = false;
}

void ConfigurationProfile::reset()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    if (!isInitialized())
        return;

    Q_ASSERT(!m_isInBatchUpdate);
    m_settings->sync();
    m_name.clear();
    m_path.clear();
    m_settings.reset();
}

}  // namespace SettingsScope
