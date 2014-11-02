/*
 * LedDeviceLightpack.hpp
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


#include <QIODevice>
#include <QTimer>

#include "AbstractLedDevice.hpp"
#include "TimeEvaluations.hpp"

class LedDeviceLightpack : public AbstractLedDevice {
    Q_OBJECT
public:
    LedDeviceLightpack(QObject *parent = 0);
    virtual ~LedDeviceLightpack();

public slots:
    virtual const QString name() const { return "lightpack"; }
    virtual void open();
    virtual void close();
    virtual void setColors(const QList<QRgb> & colors);
    virtual void switchOffLeds();
    virtual void setRefreshDelay(int value);
    virtual void setColorDepth(int value);
    virtual void setSmoothSlowdown(int value);
    virtual void setColorSequence(QString /*value*/);
    virtual void requestFirmwareVersion();
    virtual void updateDeviceSettings();
    virtual size_t maxLedsCount();
    virtual size_t defaultLedsCount() { return maxLedsCount(); }
    size_t lightpacksFound() { return m_devices.size(); }

private slots:
    void restartPingDevice(bool isSuccess);
    void timerPingDeviceTimeout();

private:
    bool readDataFromDevice();
    bool writeBufferToDevice(int command, QIODevice *device, bool tryAgain = true);
    bool tryToReopenDevice();
    bool readDataFromDeviceWithCheck();
    bool writeBufferToDeviceWithCheck(int command, QIODevice *device);
    void resizeColorsBuffer(int buffSize);
    void closeDevices();
    void open(unsigned short vid, unsigned short pid);

    QTimer m_timerPingDevice;
    QList<QIODevice*> m_devices;
    unsigned char m_readBuffer[65];    /* 0-ReportID, 1..65-data */
    unsigned char m_writeBuffer[65];   /* 0-ReportID, 1..65-data */
};
