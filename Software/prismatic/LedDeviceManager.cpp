/*
 * LedDeviceManager.cpp
 *
 *  Created on: 17.04.2011
 *      Author: Timur Sattarov && Mike Shatohin
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
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

#include <qglobal.h>

#include "LedDeviceManager.hpp"
#include "devices/LedDeviceLightpack.hpp"

#ifdef Q_OS_WIN
#include "devices/LedDeviceAlienFx.hpp"
#endif

#include "devices/LedDeviceAdalight.hpp"
#include "devices/LedDeviceArdulight.hpp"
#include "devices/LedDeviceVirtual.hpp"
#include "Settings.hpp"
#include "third_party/qtutils/include/QTUtils.hpp"

using namespace SettingsScope;

LedDeviceManager::LedDeviceManager(QObject *parent)
    : QObject(parent)
    , m_isLastCommandCompleted(true)
    , m_isColorsSaved(false)
    , m_backlightStatus(Backlight::StatusOn)
    , m_ledDevice(CURRENT_LOCATION)
    , m_cmdTimeoutTimer(NULL)
{
    for (int i = 0; i < SupportedDevices::DeviceTypesCount; i++)
        m_ledDevices.append(NULL);

    m_settings = SettingsReader::instance();
}

LedDeviceManager::~LedDeviceManager()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    for (int i = 0; i < m_ledDevices.size(); i++) {
        if(m_ledDevices[i])
            m_ledDevices[i]->close();

        if (m_ledDevices[i] != m_ledDevice.get())
            delete m_ledDevices[i];
    }
    const bool joined = m_ledDevice.join(1000);
    Q_ASSERT(joined);
    Q_UNUSED(joined);

    if (m_cmdTimeoutTimer)
        delete m_cmdTimeoutTimer;
}

void LedDeviceManager::init()
{
    if (!m_cmdTimeoutTimer)
        m_cmdTimeoutTimer = new QTimer();

    m_cmdTimeoutTimer->setInterval(100);
    connect(m_cmdTimeoutTimer, SIGNAL(timeout()), this, SLOT(ledDeviceCommandTimedOut()));

    initLedDevice();
}

void LedDeviceManager::recreateLedDevice(const SupportedDevices::DeviceType deviceType)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    Q_UNUSED(deviceType)

    disconnectCurrentLedDevice();

    initLedDevice();
}

void LedDeviceManager::switchOnLeds()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    m_backlightStatus = Backlight::StatusOn;
    if (m_isColorsSaved)
        emit ledDeviceSetColors(m_savedColors);
}

void LedDeviceManager::setColors(const QList<QRgb> & colors)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << "Is last command completed:" << m_isLastCommandCompleted
                    << " m_backlightStatus = " << m_backlightStatus;

    if (m_backlightStatus == Backlight::StatusOn)
    {
        m_savedColors = colors;
        m_isColorsSaved = true;
        if (m_isLastCommandCompleted)
        {
            m_cmdTimeoutTimer->start();
            m_isLastCommandCompleted = false;
            emit ledDeviceSetColors(colors);
        } else {
            cmdQueueAppend(LedDeviceCommands::SetColors);
        }
    }
}

void LedDeviceManager::switchOffLeds()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_cmdTimeoutTimer->start();
        m_isLastCommandCompleted = false;
        processOffLeds();
    } else {
        cmdQueueAppend(LedDeviceCommands::OffLeds);
    }
}

void LedDeviceManager::processOffLeds()
{
    m_backlightStatus = Backlight::StatusOff;

    emit ledDeviceOffLeds();
}

void LedDeviceManager::setRefreshDelay(int value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value
                    << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_cmdTimeoutTimer->start();
        m_isLastCommandCompleted = false;
        emit ledDeviceSetRefreshDelay(value);
    } else {
        m_savedRefreshDelay = value;
        cmdQueueAppend(LedDeviceCommands::SetRefreshDelay);
    }
}

void LedDeviceManager::setColorDepth(int value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_cmdTimeoutTimer->start();
        m_isLastCommandCompleted = false;
        emit ledDeviceSetColorDepth(value);
    } else {
        m_savedColorDepth = value;
        cmdQueueAppend(LedDeviceCommands::SetColorDepth);
    }
}

void LedDeviceManager::setSmoothSlowdown(int value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceSetSmoothSlowdown(value);
    } else {
        m_savedSmoothSlowdown = value;
        cmdQueueAppend(LedDeviceCommands::SetSmoothSlowdown);
    }
}

void LedDeviceManager::setGamma(double value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceSetGamma(value);
    } else {
        m_savedGamma = value;
        cmdQueueAppend(LedDeviceCommands::SetGamma);
    }
}

void LedDeviceManager::setBrightness(int value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceSetBrightness(value);
    } else {
        m_savedBrightness = value;
        cmdQueueAppend(LedDeviceCommands::SetBrightness);
    }
}

void LedDeviceManager::setLuminosityThreshold(int value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceSetLuminosityThreshold(value);
    } else {
        m_savedLuminosityThreshold = value;
        cmdQueueAppend(LedDeviceCommands::SetLuminosityThreshold);
    }
}

void LedDeviceManager::setMinimumLuminosityEnabled(bool value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceSetMinimumLuminosityEnabled(value);
    } else {
        m_savedIsMinimumLuminosityEnabled = value;
        cmdQueueAppend(LedDeviceCommands::SetMinimumLuminosityEnabled);
    }
}

void LedDeviceManager::setColorSequence(QString value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceSetColorSequence(value);
    } else {
        m_savedColorSequence = value;
        cmdQueueAppend(LedDeviceCommands::SetColorSequence);
    }
}

void LedDeviceManager::requestFirmwareVersion()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceRequestFirmwareVersion();
    } else {
        cmdQueueAppend(LedDeviceCommands::RequestFirmwareVersion);
    }
}

void LedDeviceManager::updateDeviceSettings()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceUpdateDeviceSettings();
    } else {
        cmdQueueAppend(LedDeviceCommands::UpdateDeviceSettings);
    }
}

void LedDeviceManager::updateWBAdjustments()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << "Is last command completed:" << m_isLastCommandCompleted;

    if (m_isLastCommandCompleted)
    {
        m_isLastCommandCompleted = false;
        m_cmdTimeoutTimer->start();
        emit ledDeviceUpdateWBAdjustments();
    } else {
        cmdQueueAppend(LedDeviceCommands::UpdateWBAdjustments);
    }
}

void LedDeviceManager::ledDeviceCommandCompleted(bool ok)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << ok;

    m_cmdTimeoutTimer->stop();

    if (ok)
    {
        if (m_cmdQueue.isEmpty() == false)
            cmdQueueProcessNext();
        else
            m_isLastCommandCompleted = true;
    }
    else
    {
        m_cmdQueue.clear();
        m_isLastCommandCompleted = true;
    }

    emit ioDeviceSuccess(ok);
}

void LedDeviceManager::initLedDevice()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_isLastCommandCompleted = true;

    SupportedDevices::DeviceType connectedDevice = m_settings->getConnectedDevice();

    if (m_ledDevices[connectedDevice] == NULL)
    {
        m_ledDevices[connectedDevice] = createLedDevice(connectedDevice);
        connectLedDevice(m_ledDevices[connectedDevice]);
    } else {
        Q_ASSERT(m_ledDevices[connectedDevice] != m_ledDevice.get());
        disconnectCurrentLedDevice();
        connectLedDevice(m_ledDevices[connectedDevice]);
    }
    emit ledDeviceUpdateDeviceSettings();
    emit ledDeviceOpen();
}

AbstractLedDevice * LedDeviceManager::createLedDevice(SupportedDevices::DeviceType deviceType)
{
    using namespace SupportedDevices;

#if !defined(Q_OS_WIN)
    if (deviceType == DeviceTypeAlienFx) {
        qWarning() << Q_FUNC_INFO << "AlienFx not supported on current platform";

        Settings::instance()->setConnectedDevice(DefaultDeviceType);
        deviceType = Settings::instance()->getConnectedDevice();
    }
#endif /* Q_OS_WIN */

    switch (deviceType) {
    case DeviceTypeLightpack:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::LightpackDevice";
        return new LedDeviceLightpack();

    case DeviceTypeAlienFx:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::AlienFxDevice";

#       ifdef Q_OS_WIN
        return new LedDeviceAlienFx();
#       else
        break;
#       endif /* Q_OS_WIN */

    case DeviceTypeAdalight:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::AdalightDevice";
        return new LedDeviceAdalight(
            m_settings->getAdalightSerialPortName(),
            m_settings->getAdalightSerialPortBaudRate());

    case DeviceTypeArdulight:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::ArdulightDevice";
        return new LedDeviceArdulight(
            m_settings->getArdulightSerialPortName(),
            m_settings->getArdulightSerialPortBaudRate());

    case DeviceTypeVirtual:
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "SupportedDevices::VirtualDevice";
        return new LedDeviceVirtual(m_settings->getDeviceGamma(),
                                    m_settings->getDeviceBrightness());

    default:
        break;
    }

    qFatal("%s %s%d%s", Q_FUNC_INFO,
           "Create LedDevice fail. deviceType = '", deviceType,
           "'. Application exit.");

    return NULL; // Avoid compiler warning
}

void LedDeviceManager::connectLedDevice(AbstractLedDevice * device) {
    if (device == NULL) {
        qWarning() << Q_FUNC_INFO << "device == NULL";
        return;
    }

    QtUtils::makeQueuedConnector(device, this)
        .connect(SIGNAL(commandCompleted(bool)), SLOT(ledDeviceCommandCompleted(bool)))
        .connect(SIGNAL(firmwareVersion(QString)), SIGNAL(firmwareVersion(QString)))
        .connect(SIGNAL(ioDeviceSuccess(bool)), SIGNAL(ioDeviceSuccess(bool)))
        .connect(SIGNAL(openDeviceSuccess(bool)), SIGNAL(openDeviceSuccess(bool)))
        .connect(SIGNAL(colorsUpdated(QList<QRgb>)),
                 SIGNAL(setColors_VirtualDeviceCallback(QList<QRgb>)));

    QtUtils::makeQueuedConnector(this, device)
        .connect(SIGNAL(ledDeviceOpen()), SLOT(open()))
        .connect(SIGNAL(ledDeviceSetColors(QList<QRgb>)), SLOT(setColors(QList<QRgb>)))
        .connect(SIGNAL(ledDeviceOffLeds()), SLOT(switchOffLeds()))
        .connect(SIGNAL(ledDeviceSetRefreshDelay(int)), SLOT(setRefreshDelay(int)))
        .connect(SIGNAL(ledDeviceSetColorDepth(int)), SLOT(setColorDepth(int)))
        .connect(SIGNAL(ledDeviceSetSmoothSlowdown(int)), SLOT(setSmoothSlowdown(int)))
        .connect(SIGNAL(ledDeviceSetGamma(double)), SLOT(setGamma(double)))
        .connect(SIGNAL(ledDeviceSetBrightness(int)), SLOT(setBrightness(int)))
        .connect(SIGNAL(ledDeviceSetColorSequence(QString)), SLOT(setColorSequence(QString)))
        .connect(SIGNAL(ledDeviceSetLuminosityThreshold(int)) ,SLOT(setLuminosityThreshold(int)))
        .connect(SIGNAL(ledDeviceSetMinimumLuminosityEnabled(bool)),
                 SLOT(setMinimumLuminosityThresholdEnabled(bool)))
        .connect(SIGNAL(ledDeviceRequestFirmwareVersion()), SLOT(requestFirmwareVersion()))
        .connect(SIGNAL(ledDeviceUpdateWBAdjustments()), SLOT(updateDeviceSettings()))
        .connect(SIGNAL(ledDeviceUpdateDeviceSettings()), SLOT(updateDeviceSettings()));

    m_ledDevice.init(device);
}

void LedDeviceManager::disconnectCurrentLedDevice()
{
    const AbstractLedDevice* const device = m_ledDevice.get();
    if (device == NULL)
    {
        qWarning() << Q_FUNC_INFO << "device == NULL";
        return;
    }

    QtUtils::makeConnector(device, this)
        .disconnect(SIGNAL(commandCompleted(bool)), SLOT(ledDeviceCommandCompleted(bool)))
        .disconnect(SIGNAL(firmwareVersion(QString)), SIGNAL(firmwareVersion(QString)))
        .disconnect(SIGNAL(ioDeviceSuccess(bool)), SIGNAL(ioDeviceSuccess(bool)))
        .disconnect(SIGNAL(openDeviceSuccess(bool)), SIGNAL(openDeviceSuccess(bool)))
        .disconnect(SIGNAL(colorsUpdated(QList<QRgb>)),
                    SIGNAL(setColors_VirtualDeviceCallback(QList<QRgb>)));

    QtUtils::makeConnector(this, device)
        .disconnect(SIGNAL(ledDeviceOpen()), SLOT(open()))
        .disconnect(SIGNAL(ledDeviceSetColors(QList<QRgb>)), SLOT(setColors(QList<QRgb>)))
        .disconnect(SIGNAL(ledDeviceOffLeds()), SLOT(switchOffLeds()))
        .disconnect(SIGNAL(ledDeviceSetRefreshDelay(int)), SLOT(setRefreshDelay(int)))
        .disconnect(SIGNAL(ledDeviceSetColorDepth(int)), SLOT(setColorDepth(int)))
        .disconnect(SIGNAL(ledDeviceSetSmoothSlowdown(int)), SLOT(setSmoothSlowdown(int)))
        .disconnect(SIGNAL(ledDeviceSetGamma(double)), SLOT(setGamma(double)))
        .disconnect(SIGNAL(ledDeviceSetBrightness(int)), SLOT(setBrightness(int)))
        .disconnect(SIGNAL(ledDeviceSetColorSequence(QString)), SLOT(setColorSequence(QString)))
        .disconnect(SIGNAL(ledDeviceSetLuminosityThreshold(int)), SLOT(setLuminosityThreshold(int)))
        .disconnect(SIGNAL(ledDeviceSetMinimumLuminosityEnabled(bool)),
                    SLOT(setMinimumLuminosityThresholdEnabled(bool)))
        .disconnect(SIGNAL(ledDeviceRequestFirmwareVersion()), SLOT(requestFirmwareVersion()))
        .disconnect(SIGNAL(ledDeviceUpdateWBAdjustments()), SLOT(updateDeviceSettings()))
        .disconnect(SIGNAL(ledDeviceUpdateDeviceSettings()), SLOT(updateDeviceSettings()));
}

void LedDeviceManager::cmdQueueAppend(LedDeviceCommands::Cmd cmd)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << cmd;

    if (m_cmdQueue.contains(cmd) == false)
    {
        m_cmdQueue.append(cmd);
    }
}

void LedDeviceManager::cmdQueueProcessNext()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << m_cmdQueue;

    if (m_cmdQueue.isEmpty() == false)
    {
        LedDeviceCommands::Cmd cmd = m_cmdQueue.takeFirst();

        DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "processing cmd = " << cmd;

        switch(cmd)
        {
        case LedDeviceCommands::OffLeds:
            m_cmdTimeoutTimer->start();
            processOffLeds();
            break;

        case LedDeviceCommands::SetColors:
            if (m_isColorsSaved) {
                m_cmdTimeoutTimer->start();
                emit ledDeviceSetColors(m_savedColors);
            }
            break;

        case LedDeviceCommands::SetRefreshDelay:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetRefreshDelay(m_savedRefreshDelay);
            break;

        case LedDeviceCommands::SetColorDepth:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetColorDepth(m_savedColorDepth);
            break;

        case LedDeviceCommands::SetSmoothSlowdown:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetSmoothSlowdown(m_savedSmoothSlowdown);
            break;

        case LedDeviceCommands::SetGamma:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetGamma(m_savedGamma);
            break;

        case LedDeviceCommands::SetBrightness:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetBrightness(m_savedBrightness);
            break;

        case LedDeviceCommands::SetColorSequence:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetColorSequence(m_savedColorSequence);
            break;

        case LedDeviceCommands::SetLuminosityThreshold:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetLuminosityThreshold(m_savedLuminosityThreshold);
            break;

        case LedDeviceCommands::SetMinimumLuminosityEnabled:
            m_cmdTimeoutTimer->start();
            emit ledDeviceSetMinimumLuminosityEnabled(m_savedIsMinimumLuminosityEnabled);
            break;

        case LedDeviceCommands::RequestFirmwareVersion:
            m_cmdTimeoutTimer->start();
            emit ledDeviceRequestFirmwareVersion();
            break;

        case LedDeviceCommands::UpdateDeviceSettings:
            m_cmdTimeoutTimer->start();
            emit ledDeviceUpdateDeviceSettings();
            break;

        case LedDeviceCommands::UpdateWBAdjustments:
            m_cmdTimeoutTimer->start();
            emit ledDeviceUpdateWBAdjustments();
            break;

        default:
            qCritical() << Q_FUNC_INFO << "fail process cmd =" << cmd;
            break;
        }
    }
}

void LedDeviceManager::ledDeviceCommandTimedOut()
{
    ledDeviceCommandCompleted(false);
    emit ioDeviceSuccess(false);
}
