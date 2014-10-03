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

#ifndef DEVICETYPESINFO_HPP
#define DEVICETYPESINFO_HPP

#include <QMap>
#include <QString>

#include "enums.hpp"

namespace SettingsScope {

class DeviceTypesInfo {
public:
    DeviceTypesInfo(const QString& defaultDeviceName);

    void initDevicesMap();
    bool supportsDevice(const QString& deviceName) const;
    SupportedDevices::DeviceType getDeviceType(const QString& deviceName) const;
    QString getDeviceName(SupportedDevices::DeviceType deviceType) const;
    QString getDeviceKeyNumberOfLeds(SupportedDevices::DeviceType deviceType) const;
    void addDeviceType(SupportedDevices::DeviceType deviceType,
                       const QString& deviceName,
                       const QString& deviceKeyNumberOfLeds);

private:
    struct NameAndKeyNumberOfLeds {
        NameAndKeyNumberOfLeds() {}

        NameAndKeyNumberOfLeds(const QString& n, const QString& leds)
            : name(n), keyNumberOfLeds(leds) {
        }

        QString name;
        QString keyNumberOfLeds;
    };

    typedef QMap<SupportedDevices::DeviceType, NameAndKeyNumberOfLeds> DevicesMap;
    DevicesMap m_deviceNamesAndLeds;
    QMap<QString, SupportedDevices::DeviceType> m_devicesNameToTypeMap;
    const QString m_defaultDeviceName;
};

}  // namespace SettingsScope

#endif // DEVICETYPESINFO_HPP
