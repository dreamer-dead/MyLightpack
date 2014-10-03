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

#ifndef SETTINGSREADER_HPP
#define SETTINGSREADER_HPP

#include <QColor>
#include <QString>

#include "enums.hpp"
#include "types.h"
#include "BaseVersion.hpp"

namespace SettingsScope {

class SettingsProfiles;
class DeviceTypesInfo;

class SettingsReader {
public:
    static SettingsReader * instance();

    BaseVersion getVersion() const;

    // Main
    QString getLanguage() const;
    int getDebugLevel() const;
    bool isApiEnabled() const;
    bool isListenOnlyOnLoInterface() const;
    int getApiPort() const;
    QString getApiAuthKey() const;
    bool isApiAuthEnabled() const;
    bool isExpertModeEnabled() const;
    bool isKeepLightsOnAfterExit() const;
    bool isKeepLightsOnAfterLock() const;
    bool isPingDeviceEverySecond() const;
    bool isUpdateFirmwareMessageShown() const;
    SupportedDevices::DeviceType getConnectedDevice() const;
    QString getConnectedDeviceName() const;
    QKeySequence getHotkey(const QString &actionName) const;
    QString getAdalightSerialPortName() const;
    int getAdalightSerialPortBaudRate() const;
    QString getArdulightSerialPortName() const;
    int getArdulightSerialPortBaudRate() const;
    bool isConnectedDeviceUsesSerialPort() const;
    // [Adalight | Ardulight | Lightpack | ... | Virtual]
    int getNumberOfLeds(SupportedDevices::DeviceType device) const;
    int getNumberOfConnectedDeviceLeds() const;
    QString getColorSequence(SupportedDevices::DeviceType device) const;
    QString getLastProfileName() const;

    // Profile
    int getGrabSlowdown() const;
    bool isBacklightEnabled() const;
    bool isGrabAvgColorsEnabled() const;
    bool isSendDataOnlyIfColorsChanges() const;
    int getLuminosityThreshold() const;
    bool isMinimumLuminosityEnabled() const;

    // [Device]
    int getDeviceRefreshDelay() const;
    int getDeviceBrightness() const;
    int getDeviceSmooth() const;
    int getDeviceColorDepth() const;
    double getDeviceGamma() const;

    Grab::GrabberType getGrabberType() const;

#ifdef D3D10_GRAB_SUPPORT
    bool isDx1011GrabberEnabled() const;
#endif

    Lightpack::Mode getLightpackMode();
    bool isMoodLampLiquidMode() const;
    QColor getMoodLampColor() const;
    int getMoodLampSpeed() const;

    QList<WBAdjustment> getLedCoefs() const;

    double getLedCoefRed(int ledIndex);
    double getLedCoefGreen(int ledIndex);
    double getLedCoefBlue(int ledIndex);

    QSize getLedSize(int ledIndex) const;
    QPoint getLedPosition(int ledIndex) const;
    bool isLedEnabled(int ledIndex) const;

    uint getLastReadUpdateId() const;

protected:
    SettingsReader(SettingsProfiles& profiles,
                   const DeviceTypesInfo& deviceTypes)
        : m_profiles(profiles)
        , m_deviceTypes(deviceTypes) {
    }

    ~SettingsReader() {}

    double getValidLedCoef(int ledIndex, const QString & keyCoef);

    SettingsProfiles& m_profiles;
    const DeviceTypesInfo& m_deviceTypes;
};

}  // namespace SettingsScope

#endif // SETTINGSREADER_HPP
