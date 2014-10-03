/*
 * SettingsSource.hpp
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

#ifndef SETTINGSSOURCE_HPP
#define SETTINGSSOURCE_HPP

#include <QString>
#include <QVariant>

namespace SettingsScope {

class SettingsSource
{
public:
    virtual QVariant value(const QString & key) const = 0;
    virtual void setValue(const QString & key, const QVariant & value) = 0;
    virtual bool contains(const QString& key) const = 0;
    virtual void remove(const QString& key) = 0;
    virtual void sync() {}
    virtual ~SettingsSource() {}
};

}  // namespace SettingsScope

#endif // SETTINGSSOURCE_HPP
