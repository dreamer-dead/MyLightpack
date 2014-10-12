#include "PrismatikMath.hpp"
#include "gtest/gtest.h"

TEST(LightpackMathTest, HSV) {
    namespace PM = PrismatikMath;

    const QRgb color = qRgb(215, 122, 0);
    EXPECT_EQ(215, PM::getValueHSV(color)) << "getValueHSV() is incorrect";

    const QRgb testRgb = qRgb(200, 100, 0);
    EXPECT_EQ(qRgb(200, 150, 100), PM::withChromaHSV(testRgb, 100))
        << "getChromaHSV() is incorrect";
    EXPECT_EQ(qRgb(200, 75, 0), PM::withChromaHSV(testRgb, 250))
        << "getChromaHSV() is incorrect";

    EXPECT_EQ(testRgb, PM::withChromaHSV(testRgb, PM::getChromaHSV(testRgb)))
        << "getChromaHSV() is incorrect";
}
