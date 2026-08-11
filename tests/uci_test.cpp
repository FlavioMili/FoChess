#include <gtest/gtest.h>

#include "uci.h"

TEST(UCITest, TimeAllocationHandlesShortClocks) {
  EXPECT_EQ(UCI::allocate_time(100, 0, 20), 5);
  EXPECT_EQ(UCI::allocate_time(10, 0, 30), 1);
  EXPECT_EQ(UCI::allocate_time(1, 0, 30), 1);
}

TEST(UCITest, TimeAllocationUsesMovesToGoAndIncrement) {
  EXPECT_EQ(UCI::allocate_time(60000, 0, 30), 2000);
  EXPECT_EQ(UCI::allocate_time(60000, 1000, 10), 6500);
  EXPECT_EQ(UCI::allocate_time(0, 1000, 10), 0);
}
