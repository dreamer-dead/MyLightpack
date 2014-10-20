#include <QObject>

#include "gtest/gtest.h"
#include "mocks/SignalAndSlotObject.hpp"
#include "third_party/qtutils/include/QTUtils.hpp"
#include "third_party/qtutils/include/RegisteredThread.hpp"
#include "third_party/qtutils/include/ThreadedObject.hpp"

TEST(RegisteredThreadTests, RegisteredThreadsCount) {
    EXPECT_EQ(0u, QtUtils::registeredThreadsCount());
    const QThread* const thread = NULL;
    EXPECT_EQ(1u, QtUtils::registerThread(thread, "here", 0u));
    QtUtils::deregisterThread(thread);
    EXPECT_EQ(0u, QtUtils::registeredThreadsCount());
}

TEST(RegisteredThreadTests, RegisteredThreadLifecycle) {
    EXPECT_EQ(0u, QtUtils::registeredThreadsCount());
    {
        QtUtils::RegisteredThread thread("here", 0);
        Q_UNUSED(thread);
        EXPECT_EQ(1u, QtUtils::registeredThreadsCount());
    }
    EXPECT_EQ(0u, QtUtils::registeredThreadsCount());
}

TEST(ThreadedObjectTests, CreateObject) {
    SignalAndSlotObject::State state;
    SignalAndSlotObject* signalHandler = new SignalAndSlotObject(state);
    QtUtils::ThreadedObject<SignalAndSlotObject> object;
    EXPECT_EQ(static_cast<SignalAndSlotObject*>(NULL), object.get());
    object.init(signalHandler);

    EXPECT_EQ(signalHandler, object.get());
    EXPECT_EQ(signalHandler, object.operator->());
    EXPECT_EQ(&object.thread(), signalHandler->thread());
    EXPECT_TRUE(object.thread().isRunning());

    EXPECT_FALSE(state.wasDeleted);
    // To avoid asserting in ~ThreadedObject() we should join the thread.
    EXPECT_TRUE(object.join(500));
    EXPECT_EQ(static_cast<SignalAndSlotObject*>(NULL), object.get());
    EXPECT_FALSE(object.thread().isRunning());
    EXPECT_TRUE(object.thread().isFinished());
    EXPECT_TRUE(state.wasDeleted);
}

