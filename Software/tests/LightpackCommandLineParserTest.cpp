#include "LightpackCommandLineParser.hpp"
#include "gtest/gtest.h"

TEST(LightpackCommandLineParserTest, parseVersion) {
    LightpackCommandLineParser parser;
    const QStringList arguments = QStringList() << "app.binary" << "--version";

    EXPECT_TRUE(parser.parse(arguments));
    EXPECT_TRUE(parser.isSetVersion());

    const QStringList arguments2 = QStringList() << "app.binary" << "-v";
    EXPECT_TRUE(parser.parse(arguments2));
    EXPECT_TRUE(parser.isSetVersion());
}

TEST(LightpackCommandLineParserTest, parseHelp) {
    LightpackCommandLineParser parser;
    const QStringList arguments = QStringList() << "app.binary" << "--help";

    EXPECT_TRUE(parser.parse(arguments));
    EXPECT_TRUE(parser.isSetHelp());

    const QStringList arguments2 = QStringList() << "app.binary" << "-h";
    EXPECT_TRUE(parser.parse(arguments2));
    EXPECT_TRUE(parser.isSetHelp());
}

TEST(LightpackCommandLineParserTest, parseWizard) {
    LightpackCommandLineParser parser;
    const QStringList arguments = QStringList() << "app.binary" << "--wizard";

    EXPECT_TRUE(parser.parse(arguments));
    EXPECT_TRUE(parser.isSetWizard());
}

TEST(LightpackCommandLineParserTest, parseBacklightOff) {
    LightpackCommandLineParser parser;
    const QStringList arguments = QStringList() << "app.binary" << "--off";

    EXPECT_TRUE(parser.parse(arguments));
    EXPECT_TRUE(parser.isSetBacklightOff());
}

TEST(LightpackCommandLineParserTest, parseBacklightOn) {
    LightpackCommandLineParser parser;
    const QStringList arguments = QStringList() << "app.binary" << "--on";

    EXPECT_TRUE(parser.parse(arguments));
    EXPECT_TRUE(parser.isSetBacklightOn());
}

TEST(LightpackCommandLineParserTest, parseBacklightOnAndOff) {
    LightpackCommandLineParser parser;
    const QStringList arguments = QStringList() << "app.binary" << "--on" << "--off";

    EXPECT_TRUE(!parser.parse(arguments));
    EXPECT_TRUE(parser.isSetBacklightOn());
    EXPECT_TRUE(parser.isSetBacklightOff());
}

TEST(LightpackCommandLineParserTest, parseDebuglevel) {
    const QString levelNames[] = {
        QString("high"), QString("mid"), QString("low"), QString("zero")
    };
    const Debug::DebugLevels levelValues[] = {
        Debug::HighLevel, Debug::MidLevel, Debug::LowLevel, Debug::ZeroLevel
    };

    EXPECT_EQ(sizeof(levelNames)/sizeof(levelNames[0]), sizeof(levelValues)/sizeof(levelValues[0]));
    for (size_t i = 0; i < sizeof(levelNames)/sizeof(levelNames[0]); ++i)
    {
        LightpackCommandLineParser parser;
        QStringList arguments = QStringList() << "app.binary" << QString("--debug=") + levelNames[i];
        EXPECT_TRUE(parser.parse(arguments));
        EXPECT_TRUE(parser.isSetDebuglevel());
        EXPECT_EQ(parser.debugLevel(), levelValues[i]);
    }

    for (size_t i = 0; i < sizeof(levelNames)/sizeof(levelNames[0]); ++i)
    {
        LightpackCommandLineParser parser;
        QStringList arguments = QStringList() << "app.binary" << QString("--debug-") + levelNames[i];
        EXPECT_TRUE(parser.parse(arguments));
        EXPECT_TRUE(parser.isSetDebuglevel());
        EXPECT_EQ(parser.debugLevel(), levelValues[i]);
    }
}
