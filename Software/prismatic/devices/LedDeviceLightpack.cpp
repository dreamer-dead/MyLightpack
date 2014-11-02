/*
 * LightpackDevice.cpp
 *
 *  Created on: 26.07.2010
 *      Author: Mike Shatohin (brunql)
 *     Project: Lightpack
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

#include "LedDeviceLightpack.hpp"

#include <algorithm>
#include <QApplication>
#include <QtDebug>

#include "BaseVersion.hpp"
#include "PrismatikMath.hpp"
#include "SettingsReader.hpp"
#include "common/DebugOut.hpp"

#include "../../CommonHeaders/USB_ID.h"     /* For device VID, PID, vendor name and product name */
#include "../../CommonHeaders/COMMANDS.h"   /* CMD defines */
#include "hidapi.h" /* USB HID API */

// This defines using in all data transfers to determine indexes in write_buffer[]
// In device COMMAND have index 0, data 1 and so on, report id isn't using
#define WRITE_BUFFER_INDEX_REPORT_ID    0
#define WRITE_BUFFER_INDEX_COMMAND      1
#define WRITE_BUFFER_INDEX_DATA_START   2

using namespace SettingsScope;

namespace {
const int kPingDeviceInterval = 1000;
const int kLedsPerDevice = 10;

class LightpackIO : public QIODevice {
    Q_OBJECT
public:
    static bool openDevices(unsigned short vid,
                            unsigned short pid,
                            QList<QIODevice*>* devices);

    LightpackIO(hid_device* device);
    virtual ~LightpackIO();
private:

    virtual bool isSequential() const override;
    virtual bool open(OpenMode) override;
    virtual void close() override;
    virtual qint64 readData(char *data, qint64 maxlen) override;
    virtual qint64 writeData(const char *data, qint64 len) override;

    hid_device* m_device;
};

LightpackIO::LightpackIO(hid_device* device)
    : m_device(device) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << device;
}

// static
bool LightpackIO::openDevices(
        unsigned short vid,
        unsigned short pid,
        QList<QIODevice*>* devices) {
    Q_ASSERT(devices);
    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(vid, pid);
    cur_dev = devs;
    while (cur_dev) {
        if (cur_dev->path) {
            // Open the device.
            hid_device * const handle = hid_open_path(cur_dev->path);
            if (handle != NULL) {
                // Immediately return from hid_read() if no data available.
                hid_set_nonblocking(handle, 1);
                if(cur_dev->serial_number != NULL && wcslen(cur_dev->serial_number) > 0) {
                    DEBUG_LOW_LEVEL
                        << "found Lightpack, serial number: "
                        << QString::fromWCharArray(cur_dev->serial_number);
                } else {
                    DEBUG_LOW_LEVEL << "found Lightpack, without serial number";
                }
                *devices << new LightpackIO(handle);
            } else {
                qCritical() << Q_FUNC_INFO << "couldn't open dev by path";
            }
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
    return devices->isEmpty();
}

LightpackIO::~LightpackIO() {
    LightpackIO::close();
}

bool LightpackIO::isSequential() const {
    return false;
}

bool LightpackIO::open(OpenMode mode) {
    if (!QIODevice::open(mode))
        return false;

    return !!m_device;
}

void LightpackIO::close() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "hid_close(...);";
    if (m_device)
        hid_close(m_device);
    m_device = nullptr;
    QIODevice::close();
}

qint64 LightpackIO::readData(char *data, qint64 maxlen) {
    if (!m_device)
        return -1;

    return hid_read(m_device, (unsigned char*)data, maxlen);
}

qint64 LightpackIO::writeData(const char *data, qint64 len) {
    if (!m_device)
        return -1;

    return hid_write(m_device, (const unsigned char*)data, len);
}
}

#include "LedDeviceLightpack.moc"

LedDeviceLightpack::LedDeviceLightpack(QObject *parent)
    : AbstractLedDevice(parent)
    , m_timerPingDevice(this)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    memset(m_writeBuffer, 0, sizeof(m_writeBuffer));
    memset(m_readBuffer, 0, sizeof(m_readBuffer));

    connect(&m_timerPingDevice, SIGNAL(timeout()), this, SLOT(timerPingDeviceTimeout()));
    connect(this, SIGNAL(ioDeviceSuccess(bool)), this, SLOT(restartPingDevice(bool)));
    connect(this, SIGNAL(openDeviceSuccess(bool)), this, SLOT(restartPingDevice(bool)));

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "initialized";
}

LedDeviceLightpack::~LedDeviceLightpack()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "hid_close(...);";
    closeDevices();
}

void LedDeviceLightpack::setColors(const QList<QRgb> & colors)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << hex << (colors.isEmpty() ? -1 : colors.first());
#if 0
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "thread id: " << this->thread()->currentThreadId();
#endif
    if (static_cast<size_t>(colors.count()) > maxLedsCount()) {
        qWarning() << Q_FUNC_INFO << "data size is greater than max leds count";

        // skip command with wrong data size
        return;
    }

    resizeColorsBuffer(colors.count());

    // Save colors for showing changes of the brightness
    m_colorsSaved = colors;

    applyColorModifications(colors, m_colorsBuffer);

    // First write_buffer[0] == 0x00 - ReportID, i have problems with using it
    // Second byte of usb buffer is command (write_buffer[1] == CMD_UPDATE_LEDS, see below)
    int buffIndex = WRITE_BUFFER_INDEX_DATA_START;

    bool ok = true;
    const int kLedRemap[] = {4, 3, 0, 1, 2, 5, 6, 7, 8, 9};
    const size_t kSizeOfLedColor = 6;

    memset(m_writeBuffer, 0, sizeof(m_writeBuffer));
    for (int i = 0; i < m_colorsBuffer.count(); i++)
    {
        StructRgb color = m_colorsBuffer[i];

        buffIndex = WRITE_BUFFER_INDEX_DATA_START + kLedRemap[i % 10] * kSizeOfLedColor;

        // Send main 8 bits for compability with existing devices
        m_writeBuffer[buffIndex++] = (color.r & 0x0FF0) >> 4;
        m_writeBuffer[buffIndex++] = (color.g & 0x0FF0) >> 4;
        m_writeBuffer[buffIndex++] = (color.b & 0x0FF0) >> 4;

        // Send over 4 bits for devices revision >= 6
        // All existing devices ignore it
        m_writeBuffer[buffIndex++] = (color.r & 0x000F);
        m_writeBuffer[buffIndex++] = (color.g & 0x000F);
        m_writeBuffer[buffIndex++] = (color.b & 0x000F);

        if ((i+1) % kLedsPerDevice == 0 || i == m_colorsBuffer.size() - 1) {
            if (!writeBufferToDeviceWithCheck(CMD_UPDATE_LEDS, m_devices[(i+kLedsPerDevice)/kLedsPerDevice - 1])) {
                ok = false;
            }
            memset(m_writeBuffer, 0, sizeof(m_writeBuffer));
            buffIndex = WRITE_BUFFER_INDEX_DATA_START;
        }
    }

    // WARNING: LedDeviceManager sends data only when the arrival of this signal
    emit commandCompleted(ok);
}

size_t LedDeviceLightpack::maxLedsCount() {
    if (m_devices.isEmpty())
        tryToReopenDevice();
    return m_devices.size() * kLedsPerDevice;
}

void LedDeviceLightpack::switchOffLeds() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    if (m_colorsSaved.empty()) {
        for (size_t i = 0; i < maxLedsCount(); ++i)
            m_colorsSaved << 0;
    } else {
        for (int i = 0; i < m_colorsSaved.count(); i++)
            m_colorsSaved[i] = 0;
    }

    m_timerPingDevice.stop();

    memset(m_writeBuffer, 0, sizeof(m_writeBuffer));

    bool ok = true;
    for(int i = 0; i < m_devices.size(); i++) {
        if (!writeBufferToDeviceWithCheck(CMD_UPDATE_LEDS, m_devices[i]))
            ok = false;
    }

    emit commandCompleted(ok);
    // Stop ping device if switchOffLeds() signal comes
}

void LedDeviceLightpack::setRefreshDelay(int value) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

    m_writeBuffer[WRITE_BUFFER_INDEX_DATA_START] = value & 0xff;
    m_writeBuffer[WRITE_BUFFER_INDEX_DATA_START+1] = (value >> 8);

    bool ok = true;
    for(int i = 0; i < m_devices.size(); i++) {
        if (!writeBufferToDeviceWithCheck(CMD_SET_TIMER_OPTIONS, m_devices[i]))
            ok = false;
    }
    emit commandCompleted(ok);
}

void LedDeviceLightpack::setColorDepth(int value) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

    m_writeBuffer[WRITE_BUFFER_INDEX_DATA_START] = (unsigned char)value;

    bool ok = true;
    for(int i = 0; i < m_devices.size(); i++) {
        if (!writeBufferToDeviceWithCheck(CMD_SET_PWM_LEVEL_MAX_VALUE, m_devices[i]))
            ok = false;
    }
    emit commandCompleted(ok);
}

void LedDeviceLightpack::setSmoothSlowdown(int value) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

    m_writeBuffer[WRITE_BUFFER_INDEX_DATA_START] = (unsigned char)value;

    bool ok = true;
    for(int i = 0; i < m_devices.size(); i++) {
        if (!writeBufferToDeviceWithCheck(CMD_SET_SMOOTH_SLOWDOWN, m_devices[i]))
            ok = false;
    }
    emit commandCompleted(ok);
}

void LedDeviceLightpack::setColorSequence(QString /*value*/) {
    emit commandCompleted(true);
}

void LedDeviceLightpack::requestFirmwareVersion() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    const bool ok = readDataFromDeviceWithCheck();
    QString fwVersion;

    // TODO: write command CMD_GET_VERSION to device
    if (ok) {
        const int fw_major = m_readBuffer[INDEX_FW_VER_MAJOR];
        const int fw_minor = m_readBuffer[INDEX_FW_VER_MINOR];
        fwVersion = BaseVersion(fw_major, fw_minor).toString();
    } else {
        fwVersion = tr("read device fail");
    }

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Version:" << fwVersion;

    emit firmwareVersion(fwVersion);
    emit commandCompleted(ok);
}

void LedDeviceLightpack::updateDeviceSettings() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << sender();

    AbstractLedDevice::updateDeviceSettings();
    setRefreshDelay(SettingsReader::instance()->getDeviceRefreshDelay());
    setColorDepth(SettingsReader::instance()->getDeviceColorDepth());
    setSmoothSlowdown(SettingsReader::instance()->getDeviceSmooth());

    requestFirmwareVersion();
}

void LedDeviceLightpack::open() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    if (!m_devices.isEmpty()) {
        emit openDeviceSuccess(true);
        return;
    }

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << QString("hid_open(0x%1, 0x%2)")
                       .arg(USB_VENDOR_ID, 4, 16, QChar('0'))
                       .arg(USB_PRODUCT_ID, 4, 16, QChar('0'));

    open(USB_VENDOR_ID, USB_PRODUCT_ID);
    open(USB_OLD_VENDOR_ID, USB_OLD_PRODUCT_ID);

    if (m_devices.isEmpty()) {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Lightpack devices not found";
        emit openDeviceSuccess(false);
        return;
    }

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Lightpack opened";

    updateDeviceSettings();

    emit openDeviceSuccess(true);
}

void LedDeviceLightpack::open(unsigned short vid, unsigned short pid) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "device count = " << m_devices.count();
    LightpackIO::openDevices(vid, pid, &m_devices);
}

void LedDeviceLightpack::close() {
    closeDevices();
}

bool LedDeviceLightpack::readDataFromDevice() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    if (m_devices.isEmpty() || !m_devices[0]) {
        Q_ASSERT(false);
        return false;
    }

    const qint64 bytesRead = m_devices[0]->read((char*)m_readBuffer, sizeof(m_readBuffer));
    const bool readSuccessfull = bytesRead >= 0;
    if(!readSuccessfull) {
        qWarning() << "Error reading data:" << bytesRead;
    }
    emit ioDeviceSuccess(readSuccessfull);
    return readSuccessfull;
}

bool LedDeviceLightpack::writeBufferToDevice(int command, QIODevice *device, bool tryAgain) {
    DEBUG_MID_LEVEL << Q_FUNC_INFO << command;
    Q_ASSERT(device);

    m_writeBuffer[WRITE_BUFFER_INDEX_REPORT_ID] = 0x00;
    m_writeBuffer[WRITE_BUFFER_INDEX_COMMAND] = command;

    qint64 bytesWritten = device->write((const char*)m_writeBuffer, sizeof(m_writeBuffer));
    bool writeSuccessfull = bytesWritten >= 0;
    if (!writeSuccessfull) {
        if (tryAgain) {
            bytesWritten = device->write((const char*)m_writeBuffer, sizeof(m_writeBuffer));
            writeSuccessfull = bytesWritten >= 0;
        }
        if (!writeSuccessfull)
            qWarning() << "Error writing data:" << bytesWritten;
    }
    emit ioDeviceSuccess(writeSuccessfull);
    return writeSuccessfull;
}

bool LedDeviceLightpack::tryToReopenDevice() {
    closeDevices();
    open();

    if (!m_devices.isEmpty()) {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Reopen success";
    }

    return !m_devices.isEmpty();
}

bool LedDeviceLightpack::readDataFromDeviceWithCheck() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    if (!m_devices.isEmpty()) {
        if (!readDataFromDevice()) {
            if (tryToReopenDevice())
                return readDataFromDevice();
            else
                return false;
        }
        return true;
    } else {
        if (tryToReopenDevice())
            return readDataFromDevice();
        else
            return false;
    }
}

bool LedDeviceLightpack::writeBufferToDeviceWithCheck(int command, QIODevice *device) {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;

    if (device != NULL) {
        if (!writeBufferToDevice(command, device)) {
            if (!writeBufferToDevice(command, device)) {
                if (tryToReopenDevice())
                    return writeBufferToDevice(command, device);
                else
                    return false;
            }
        }
        return true;
    } else {
        if (tryToReopenDevice())
            return writeBufferToDevice(command, device);
        else
            return false;
    }
}

void LedDeviceLightpack::resizeColorsBuffer(int buffSize) {
    if (m_colorsBuffer.count() == buffSize || buffSize < 0)
        return;

    m_colorsBuffer.clear();

    size_t checkedBufferSize = buffSize;
    if (checkedBufferSize > maxLedsCount()) {
        qCritical() << Q_FUNC_INFO
            << "buffSize > MaximumLedsCount"
            << checkedBufferSize << ">" << maxLedsCount();

        checkedBufferSize = maxLedsCount();
    }

    std::fill_n(std::back_inserter(m_colorsBuffer), checkedBufferSize, StructRgb());
}

void LedDeviceLightpack::closeDevices() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_timerPingDevice.stop();
    m_timerPingDevice.blockSignals(true);

    for(int i=0; i < m_devices.size(); i++) {
        m_devices[i]->close();
        m_devices[i]->deleteLater();
    }
    m_devices.clear();
}

void LedDeviceLightpack::restartPingDevice(bool isSuccess) {
    Q_UNUSED(isSuccess);

    if (SettingsReader::instance()->isBacklightEnabled() &&
        SettingsReader::instance()->isPingDeviceEverySecond()) {
        // Start ping device with PingDeviceInterval ms after last data transfer complete
        m_timerPingDevice.start(kPingDeviceInterval);
    } else {
        m_timerPingDevice.stop();
    }
}

void LedDeviceLightpack::timerPingDeviceTimeout() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;
    if (m_devices.isEmpty()) {
        DEBUG_MID_LEVEL << Q_FUNC_INFO << "open devices";
        open();

        if (m_devices.isEmpty()) {
            DEBUG_MID_LEVEL << Q_FUNC_INFO << "open devices fail";
            emit openDeviceSuccess(false);
            return;
        }
        DEBUG_MID_LEVEL << Q_FUNC_INFO << "open devices ok";

        emit openDeviceSuccess(true);
        closeDevices(); // device should be opened by open() function
        return;
    }

    DEBUG_MID_LEVEL << Q_FUNC_INFO << "hid_write";
    const bool writeSuccessfull = writeBufferToDevice(CMD_NOP, m_devices[0], false);
    if (!writeSuccessfull) {
        closeDevices();
    }

    DEBUG_MID_LEVEL << Q_FUNC_INFO << "hid_write " << (writeSuccessfull ? "ok" : "fail");
    emit ioDeviceSuccess(writeSuccessfull);
}
