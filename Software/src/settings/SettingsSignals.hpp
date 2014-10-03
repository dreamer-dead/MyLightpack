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

#ifndef SETTINGSSIGNALS_HPP
#define SETTINGSSIGNALS_HPP

#include <QColor>
#include <QObject>
#include <QString>

#include "enums.hpp"

namespace SettingsScope {

class SettingsSignals : public QObject {
    Q_OBJECT

protected:
    SettingsSignals() {}
    ~SettingsSignals() {}

signals:
    void profileLoaded(const QString &);
    void currentProfileNameChanged(const QString &);
    void currentProfileRemoved();
    void currentProfileInited(const QString &);
    void apiServerSettingsChanged();
    void apiKeyChanged(const QString &);
    void expertModeEnabledChanged(bool);
    void keepLightsOnAfterExitChanged(bool isEnabled);
    void keepLightsOnAfterLockChanged(bool isEnabled);
    void pingDeviceEverySecondEnabledChanged(bool);

    void languageChanged(const QString &);
    void debugLevelChanged(int);
    void updateFirmwareMessageShownChanged(bool isShown);
    void connectedDeviceChanged(const SupportedDevices::DeviceType device);
    void hotkeyChanged(const QString &actionName, const QKeySequence & newKeySequence, const QKeySequence &oldKeySequence);
    void adalightSerialPortNameChanged(const QString & port);
    void adalightSerialPortBaudRateChanged(const QString & baud);
    void ardulightSerialPortNameChanged(const QString & port);
    void ardulightSerialPortBaudRateChanged(const QString & baud);
    void lightpackNumberOfLedsChanged(int numberOfLeds);
    void adalightNumberOfLedsChanged(int numberOfLeds);
    void ardulightNumberOfLedsChanged(int numberOfLeds);
    void virtualNumberOfLedsChanged(int numberOfLeds);
    void grabSlowdownChanged(int value);
    void backlightEnabledChanged(bool isEnabled);
    void grabAvgColorsEnabledChanged(bool isEnabled);
    void sendDataOnlyIfColorsChangesChanged(bool isEnabled);
    void luminosityThresholdChanged(int value);
    void minimumLuminosityEnabledChanged(bool value);
    void deviceRefreshDelayChanged(int value);
    void deviceBrightnessChanged(int value);
    void deviceSmoothChanged(int value);
    void deviceColorDepthChanged(int value);
    void deviceGammaChanged(double gamma);
    void deviceColorSequenceChanged(QString value);
    void grabberTypeChanged(const Grab::GrabberType grabMode);
    void dx1011GrabberEnabledChanged(const bool isEnabled);
    void lightpackModeChanged(const Lightpack::Mode mode);
    void moodLampLiquidModeChanged(bool isLiquidMode);
    void moodLampColorChanged(const QColor color);
    void moodLampSpeedChanged(int value);
    void ledCoefRedChanged(int ledIndex, double value);
    void ledCoefGreenChanged(int ledIndex, double value);
    void ledCoefBlueChanged(int ledIndex, double value);
    void ledSizeChanged(int ledIndex, const QSize &size);
    void ledPositionChanged(int ledIndex, const QPoint &position);
    void ledEnabledChanged(int ledIndex, bool isEnabled);
};

}  // namespace SettingsScope

#endif // SETTINGSSIGNALS_HPP
