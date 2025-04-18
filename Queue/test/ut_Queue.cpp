#include <gtest/gtest.h>

#include "Queue.h"

TEST(Queue, test1) {
    int size = 3;
    Queue<int> q(size);

    q.push(0);
    ASSERT_EQ(q.pop(), 0);

    q.push(1);
    q.push(2);
    q.push(3);
    q.push(4);
    ASSERT_EQ(q.pop(), 2);
    ASSERT_EQ(q.pop(), 3);
    ASSERT_EQ(q.pop(), 4);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}