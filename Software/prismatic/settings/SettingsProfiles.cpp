#include "SettingsProfiles.hpp"

#include "common/DebugOut.hpp"

namespace SettingsScope {

void SettingsProfiles::setValueMain(const QString & key, const QVariant & value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << value;

    m_mainProfile.setValue(key, value);
}

QVariant SettingsProfiles::valueMain(const QString & key) const
{
    QVariant value = m_mainProfile.value(key);
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

QVariant SettingsProfiles::valueMain(const QString & key,
                                     const QVariant& defaultValue) const {
    QVariant value = m_mainProfile.valueOrDefault(key, defaultValue);
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

void SettingsProfiles::setValue(const QString & key, const QVariant & value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;

    m_currentProfile.setValue(key, value);
}

QVariant SettingsProfiles::value(const QString & key) const
{
    QVariant value = m_currentProfile.value(key);
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

QVariant SettingsProfiles::value(const QString & key,
                                 const QVariant& defaultValue) const {
    QVariant value = m_currentProfile.valueOrDefault(key, defaultValue);
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key << "= " << value;
    return value;
}

}  // namespace SettingsScope
