#include <QColor>
#include <QList>

#include "ApiServerSetColorTask.hpp"
#include "gtest/gtest.h"

namespace
{
template <typename T, size_t N>
char ( &_ArraySizeHelper( T (&array)[N] ))[N];
#define ARRAY_SIZE( array ) (sizeof( _ArraySizeHelper( array ) ))

struct ResultReceiver : public QObject {
    Q_OBJECT
public:
    ResultReceiver() : m_result(false) {
    }

    void resultHandler(bool result) {
        m_result = result;
    }

    void handleColors(const QList<QRgb>& colors) {
        m_colors = colors;
    }

    bool result() const {
        return m_result;
    }

    bool m_result;
    QList<QRgb> m_colors;
};
}

#include "CommandSetColorParsingTests.moc"

TEST(CommandSetColorParsingTest, SingleColorValidCommand) {
    ResultReceiver handler;
    ApiServerSetColorTask parser;
    struct ColorData {
        const char* cmd;
        int led;
        QRgb color;
    } const colorData [] = {
        // Color command without semicolon is valid!
        { "1-1,1,1",        1,  qRgb(1, 1, 1) },
        { "1-1,1,1",        1,  qRgb(1, 1, 1) },
        { "2-1,1,1",        2,  qRgb(1, 1, 1) },
        { "10-1,1,1",       10, qRgb(1, 1, 1) },
        { "1-192,1,1",      1,  qRgb(192, 1, 1) },
        { "1-1,185,1",      1,  qRgb(1, 185, 1) },
        { "1-14,15,19",     1,  qRgb(14, 15, 19) },
        { "1-255,255,255",  1,  qRgb(255, 255, 255) },
        { "10-255,255,255", 10, qRgb(255, 255, 255) },
        { "10-0,0,1",       10, qRgb(0, 0, 1) },
        { "10-1,0,0",       10, qRgb(1, 0, 0) }
    };

    QObject::connect(&parser, &ApiServerSetColorTask::taskParseSetColorIsSuccess,
                     &handler, &ResultReceiver::resultHandler);
    QObject::connect(&parser, &ApiServerSetColorTask::taskParseSetColorDone,
                     &handler, &ResultReceiver::handleColors);
    parser.setApiDeviceNumberOfLeds(20);
    for (size_t ii = 0; ii < ARRAY_SIZE(colorData); ++ii) {
        handler.m_result = false;
        emit parser.startParseSetColorTask(QByteArray(colorData[ii].cmd));
        EXPECT_TRUE(handler.result());
        ASSERT_LT(colorData[ii].led - 1, handler.m_colors.size());
        EXPECT_EQ(colorData[ii].color, handler.m_colors[colorData[ii].led - 1]);
    }
}

TEST(CommandSetColorParsingTest, MultipleColorValidCommand) {
    const char* colorStrings[] = {
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;5-5,5,5;6-6,6,6;7-7,7,7;8-8,8,8;9-9,9,9;10-10,10,10;",
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;5-5,5,5;6-6,6,6;7-7,7,7;8-8,8,8;9-9,9,9;",
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;5-5,5,5;6-6,6,6;7-7,7,7;8-8,8,8;",
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;5-5,5,5;6-6,6,6;7-7,7,7;",
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;5-5,5,5;6-6,6,6;",
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;5-5,5,5;",
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;",
        "1-1,1,1;2-2,2,2;3-3,3,3;",
        "1-1,1,1;2-2,2,2;",
        "1-1,1,1;"
    };

    ResultReceiver handler;
    ApiServerSetColorTask parser;
    QObject::connect(&parser, &ApiServerSetColorTask::taskParseSetColorIsSuccess,
                     &handler, &ResultReceiver::resultHandler);
    for (size_t ii = 0; ii < ARRAY_SIZE(colorStrings); ++ii) {
        handler.m_result = false;
        emit parser.startParseSetColorTask(QByteArray(colorStrings[ii]));
        EXPECT_TRUE(handler.result());
    }
}

TEST(CommandSetColorParsingTest, SingleColorInvalidCommand) {
    const char* colorStrings[] = {
        "1--1,1,1;",
        "1-1,,1,1;",
        "1-1,1,,1;",
        "1-1.1.1",
        "1-1,1,1;2-",
        "11-1,1,1;",
        "0-1,1,1;",
        "1-1,-1,1;",
        "1-1,1111,1;",
        "1-1,1,256;",
        "!-1,1,1;",
        "-1,1,1;",
        "1-1,1;",
        "1-1,100000000000000000000000;",
        "1-1,1,1;2-4,5,,;",
        "1-1,1,1;2-2,2,2;3-3,3,3;4-4,4,4;5-5,5,5;;6-6,6,6;7-7,7,7;8-8,8,8;9-9,9,9;"
    };

    ResultReceiver handler;
    ApiServerSetColorTask parser;
    QObject::connect(&parser, &ApiServerSetColorTask::taskParseSetColorIsSuccess,
                     &handler, &ResultReceiver::resultHandler);
    for (size_t ii = 0; ii < ARRAY_SIZE(colorStrings); ++ii) {
        handler.m_result = true;
        emit parser.startParseSetColorTask(QByteArray(colorStrings[ii]));
        EXPECT_FALSE(handler.result());
    }
}

