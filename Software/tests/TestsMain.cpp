#include <iostream>
#include <QApplication>

#include "common/DebugOut.hpp"
#include "gtest/gtest.h"

using namespace std;

unsigned g_debugLevel = Debug::LowLevel;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    cout << "Run Lightpack tests:" << endl;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
