/*
 * ApiServerSetColorTask.cpp
 *
 *  Created on: 07.09.2011
 *      Author: Mike Shatohin
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
#include <QThread>
#include <QRegExp>
#include <algorithm>
#include <cmath>

#include "ApiServerSetColorTask.hpp"
#include "PrismatikMath.hpp"
#include "SettingsDefaults.hpp"
#include "common/DebugOut.hpp"
#include "common/PrintHelpers.hpp"

namespace {
// Minimum 7 - '2-0,0,0'
static const int kMinBufferLength = 7u;

// Minimum 7 - '99-999,999,999'
static const int kMaxBufferLength = 14u;

struct IndexAndColor {
    int index;
    QRgb color;
};

int parseSingleCommand(const char* start, const char* end, IndexAndColor& result) {
    // buffer can contains only something like this:
    // 1-34,9,125
    // 2-0,255,0;3-0,255,0;6-0,255,0;
    Q_ASSERT(start != end);
    const char* delim = (const char*)memchr(start, ';', std::distance(start, end));
    int delimOffset = 1;
    // There can be no ';' delimiter if it's a last command in sequence.
    if (!delim) {
        delim = end;
        delimOffset = 0;
    }

    const int length = std::distance(start, delim);
    if (length < kMinBufferLength || length > kMaxBufferLength)
        return -1;

    const QRegExp commandRegex("(\\d{1,2})\\-(\\d{1,3})\\,(\\d{1,3})\\,(\\d{1,3})");
    if (commandRegex.indexIn(QLatin1String(start, length), 0) < 0)
        return -1;

    if (commandRegex.captureCount() < 4)
        return -1;
    bool parseResult = false;
    result.index = commandRegex.cap(1).toInt(&parseResult);
    if (!parseResult)
        return -1;

    enum BuffRgbIndexes {
        bRed, bGreen, bBlue, bSize
    };
    // Buffer for store temp red, green and blue values.
    int buffRgb[bSize] = {0, 0, 0};
    buffRgb[bRed] = commandRegex.cap(2).toInt(&parseResult);
    if (!parseResult || buffRgb[bRed] > 255)
        return -1;

    buffRgb[bGreen] = commandRegex.cap(3).toInt(&parseResult);
    if (!parseResult || buffRgb[bGreen] > 255)
        return -1;

    buffRgb[bBlue] = commandRegex.cap(4).toInt(&parseResult);
    if (!parseResult || buffRgb[bBlue] > 255)
        return -1;
    result.color = qRgb(buffRgb[bRed], buffRgb[bGreen], buffRgb[bBlue]);
    return length + delimOffset;
}

}

ApiServerSetColorTask::ApiServerSetColorTask(QObject *parent) :
    QObject(parent)
{
    m_numberOfLeds = MaximumNumberOfLeds::Default;

    reinitColorBuffers();
}

// static
bool ApiServerSetColorTask::parseCommandSequence(const QByteArray& buffer, QList<QRgb>& result) {
    const char* it = buffer.constData();
    const char* const end = buffer.constData() + buffer.size();
    IndexAndColor parsedCommand = {-1, 0};
    while (it != end) {
        // Check the buffer length.
        if (std::distance(it, end) < kMinBufferLength) {
            API_DEBUG_OUT << "error: Too small buffer.length() = " << buffer.length();
            return false;
        }
        const int pos = parseSingleCommand(it, end, parsedCommand);
        if (pos < 0) {
            API_DEBUG_OUT << "error: couldn't parse command!";
            return false;
        }

        // Convert for using in zero-based arrays
        const int ledNumber = parsedCommand.index - 1;
        if (ledNumber < 0 || ledNumber >= result.size()) {
            API_DEBUG_OUT << "error: incorrect color index = " << parsedCommand.index;
            return false;
        }

        // Save colors
        result[ledNumber] = parsedCommand.color;
        API_DEBUG_OUT
            << "result color:" << qRed(parsedCommand.color)
                               << qGreen(parsedCommand.color)
                               << qBlue(parsedCommand.color)
            << "buffer:" << QLatin1String(it, std::distance(it, end));
        it += pos;
    }
    return !buffer.isEmpty();
}

void ApiServerSetColorTask::startParseSetColorTask(const QByteArray& buffer) {
    API_DEBUG_OUT << QString(buffer) << "task thread:" << thread()->currentThreadId();

    if (!parseCommandSequence(buffer, m_colors))
    {
        API_DEBUG_OUT << "errors while reading buffer";
        emit taskParseSetColorIsSuccess(false);
    } else {
        API_DEBUG_OUT << "read setcolor buffer - ok";
        emit taskParseSetColorDone(m_colors);
        emit taskParseSetColorIsSuccess(true);
    }
}

void ApiServerSetColorTask::reinitColorBuffers()
{
    m_colors.clear();

    for (int i = 0; i < m_numberOfLeds; i++)
        m_colors << 0;
}

void ApiServerSetColorTask::setApiDeviceNumberOfLeds(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

    m_numberOfLeds = value;

    reinitColorBuffers();
}
