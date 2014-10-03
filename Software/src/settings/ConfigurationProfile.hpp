/*
 * ConfigurationProfile.hpp
 *
 *  Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2010, 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#ifndef CONFIGURATIONPROFILE_HPP
#define CONFIGURATIONPROFILE_HPP

#include <QMutex>
#include <QScopedPointer>

#include "SettingsSource.hpp"

namespace SettingsScope {

class ConfigurationProfile
{
public:
    typedef SettingsSource* (*SettingsSourceFabricFunc)(const QString&);

    static void setSourceFabric(SettingsSourceFabricFunc fabric);

    ConfigurationProfile();

    bool init(const QString& path, const QString& name);
    bool isInitialized() const { return m_settings; }
    const QString& name() const { return m_name; }
    const QString& path() const { return m_path; }

    QVariant value(const QString & key) const;
    QVariant valueOrDefault(const QString & key, const QVariant& defaultValue) const;
    bool contains(const QString& key) const;
    void setValue(const QString & key, const QVariant & value, bool force = true);
    void remove(const QString& key);

    bool beginBatchUpdate();
    void endBatchUpdate();
    void reset();

    struct ScopedBatchUpdateGuard
    {
        ConfigurationProfile& m_profile;

        ScopedBatchUpdateGuard(ConfigurationProfile& profile)
            : m_profile(profile)
        {
            m_profile.beginBatchUpdate();
        }

        ~ScopedBatchUpdateGuard() { m_profile.endBatchUpdate(); }
    };

private:
    mutable QMutex m_mutex;
    bool m_isInBatchUpdate;
    QString m_name;
    QString m_path;
    QScopedPointer<SettingsSource> m_settings;
};

}  // namespace SettingsScope

#endif // CONFIGURATIONPROFILE_HPP
