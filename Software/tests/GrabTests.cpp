#include "GrabberContext.hpp"
#include "gtest/gtest.h"

TEST(GrabTests, GrabContextTest) {
    GrabberContext context;
    EXPECT_EQ(context.buffersCount(), 0);
    EXPECT_TRUE(context.queryBuf(10));
    EXPECT_EQ(context.buffersCount(), 1);
    EXPECT_TRUE(context.queryBuf(100));
    EXPECT_EQ(context.buffersCount(), 2);
    // Should return existring buffer
    EXPECT_TRUE(context.queryBuf(50));
    // Still 2 buffers in list
    EXPECT_EQ(context.buffersCount(), 2);
    context.releaseAllBufs();
    context.freeReleasedBufs();
    EXPECT_EQ(context.buffersCount(), 0);
}
