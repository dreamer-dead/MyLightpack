#include "DeviceTypesInfo.hpp"

#include "debug.h"

namespace SettingsScope {

DeviceTypesInfo::DeviceTypesInfo(const QString& defaultDeviceName)
    : m_defaultDeviceName(defaultDeviceName) {
}

void DeviceTypesInfo::initDevicesMap() {
    using namespace SupportedDevices;

    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    if (!m_deviceNamesAndLeds.contains(DefaultDeviceType)) {
        m_deviceNamesAndLeds[DefaultDeviceType] =
            NameAndKeyNumberOfLeds(m_defaultDeviceName, QString());
    }
    for (DevicesMap::const_iterator it = m_deviceNamesAndLeds.begin();
         it != m_deviceNamesAndLeds.end(); ++it) {
        m_devicesNameToTypeMap[it.value().name] = it.key();
    }
}

void DeviceTypesInfo::addDeviceType(SupportedDevices::DeviceType deviceType,
                                    const QString& deviceName,
                                    const QString& deviceKeyNumberOfLeds) {
    m_deviceNamesAndLeds[deviceType] =
        NameAndKeyNumberOfLeds(deviceName, deviceKeyNumberOfLeds);
}

bool DeviceTypesInfo::supportsDevice(const QString& deviceName) const {
    return m_devicesNameToTypeMap.contains(deviceName);
}

SupportedDevices::DeviceType DeviceTypesInfo::getDeviceType(
    const QString& deviceName) const {
    if (!supportsDevice(deviceName))
        return SupportedDevices::DefaultDeviceType;

    return m_devicesNameToTypeMap[deviceName];
}

QString DeviceTypesInfo::getDeviceName(
    SupportedDevices::DeviceType deviceType) const {
    if (!m_deviceNamesAndLeds.contains(deviceType))
        return m_defaultDeviceName;

    const QString& deviceName = m_deviceNamesAndLeds[deviceType].name;
    Q_ASSERT(supportsDevice(deviceName));

    return deviceName;
}

QString DeviceTypesInfo::getDeviceKeyNumberOfLeds(
    SupportedDevices::DeviceType deviceType) const {
    if (!m_deviceNamesAndLeds.contains(deviceType))
        return QString();

    const NameAndKeyNumberOfLeds& device = m_deviceNamesAndLeds[deviceType];
    Q_ASSERT(supportsDevice(device.name));

    return device.keyNumberOfLeds;
}

}  // namespace SettingsScope
