/*
 * AppVersionTest.cpp
 *
 *  Created on: 4/15/2014
 *     Project: Prismatik
 *
 *  Copyright (c) 2014 tim
 *
 *  Lightpack is an open-source, USB content-driving ambient lighting
 *  hardware.
 *
 *  Prismatik is a free, open-source software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Prismatik and Lightpack files is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "UpdatesProcessor.hpp"
#include "gtest/gtest.h"

TEST(AppVersionTest, ExplicitEquality)
{
    const AppVersion a("5.10.7");
    const AppVersion b("5.10.7");
    EXPECT_EQ(a, b) << "Version explicit equality test failed";
    EXPECT_FALSE(a != b) << "Version explicit inequality test failed";
}

TEST(AppVersionTest, ImplicitEquality)
{
    const AppVersion a("5");
    const AppVersion b("5.0.0");
    EXPECT_EQ(a, b) << "Version explicit equality test failed";
    EXPECT_FALSE(a != b) << "Version implicit equality test failed";
}

TEST(AppVersionTest, FalseEquality)
{
    const AppVersion a("5.1");
    const AppVersion b("5.0.0");
    EXPECT_NE(a, b) << "Version false equality test failed";
    EXPECT_TRUE(a != b), "Version false equality test failed";
}

TEST(AppVersionTest, LT)
{
    const AppVersion a("5");
    const AppVersion b("5.1.0");
    EXPECT_LT(a, b) << "Less than test failed";
    EXPECT_FALSE(a > b) << "Less than test failed";
}

TEST(AppVersionTest, GT)
{
    const AppVersion a("6.0.0.1");
    const AppVersion b("5.1.0.2");
    EXPECT_GT(a, b) << "Greater than test failed";
    EXPECT_FALSE(a < b) << "Greater than test failed";
}
