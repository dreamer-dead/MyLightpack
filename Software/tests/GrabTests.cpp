#include "GrabTests.hpp"

#include <QTest>

#include "GrabberContext.hpp"

GrabTests::GrabTests(QObject *parent)
    : QObject(parent) {
}

void GrabTests::testCase_GrabContextTest() {
    GrabberContext context;
    QVERIFY(context.buffersCount() == 0);
    QVERIFY(context.queryBuf(10) != 0);
    QVERIFY(context.buffersCount() == 1);
    QVERIFY(context.queryBuf(100) != 0);
    QVERIFY(context.buffersCount() == 2);
    // Should return existring buffer
    QVERIFY(context.queryBuf(50) != 0);
    // Still 2 buffers in list
    QVERIFY(context.buffersCount() == 2);
    context.releaseAllBufs();
    context.freeReleasedBufs();
    QVERIFY(context.buffersCount() == 0);
}
