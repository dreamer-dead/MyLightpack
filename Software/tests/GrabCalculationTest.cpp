/*
 * tst_GrabCalculationTestTest.cpp
 *
 *  Project: Lightpack
 *
 *  Lightpack a USB content-driving ambient lighting system
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

#include <QColor>
#include <QRgb>
#include <QRect>

#include "enums.hpp"
#include "calculations.hpp"
#include "gtest/gtest.h"

TEST(GrabCalculationTest, AvgColor) {
    using Grab::Calculations::calculateAvgColor;

    QRgb result;
    unsigned char buffer[16];
    memset(buffer, 0xfa, sizeof(buffer));
    const QRgb color = calculateAvgColor(
        &result, buffer, BufferFormatArgb, sizeof(buffer), QRect(0, 0, 4, 1));
    EXPECT_EQ(QColor(0xfa, 0xfa, 0xfa, 0xfa).rgb(), color)
        << "Failure. calculateAvgColor returned wrong errorcode " << result;
}
