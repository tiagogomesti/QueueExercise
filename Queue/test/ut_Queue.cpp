#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

#include "Queue.h"

TEST(Queue, pushElementsUntilNotFull) {
    int size = 50;
    Queue<int> q(size);

    // Pushing data into queue
    for (int i = 0; i < size; i++) {
        q.push(i);
        ASSERT_EQ(q.count(), i + 1);
    }

    // Popping data from queue
    for (int i = 0; i < size; i++) {
        ASSERT_EQ(q.pop(), i);
        ASSERT_EQ(q.count(), size - (i + 1));
    }
}

TEST(Queue, pushElementsIntoFullQueue) {
    int size = 50;
    Queue<int> q(size);

    // Filling the queue
    for (int i = 0; i < size; i++) {
        q.push(i);
        ASSERT_EQ(q.count(), i + 1);
    }

    q.push(0xFFFF);  // This value should take the place of oldest element in the queue

    // Popping data from queue
    for (int i = 0; i < size; i++) {
        if (i != (size - 1)) {  // The first (size-1) elements
            ASSERT_EQ(q.pop(), i + 1);
            ASSERT_EQ(q.count(), size - (i + 1));
        } else {  // The newer value
            ASSERT_EQ(q.pop(), 0xFFFF);
            ASSERT_EQ(q.count(), size - (i + 1));
        }
    }
}

TEST(Queue, popFromEmptyQueueBeingUnblockedAfterPushedSomeElement) {
    Queue<int> q(10);
    int valuePushed = 0xC0DE;
    int timeoutMs = 100;

    std::thread queueReaderThread(
        [timeoutMs, valuePushed](Queue<int>* q) {
            std::future<int> futureResult = std::async(std::launch::async, [q, timeoutMs, valuePushed]() { return q->pop(); });
            auto status = futureResult.wait_for(std::chrono::milliseconds(timeoutMs));
            ASSERT_EQ(status, std::future_status::ready);
            ASSERT_EQ(futureResult.get(), valuePushed);
        },
        &q);

    std::thread queueWriterThread(
        [timeoutMs, valuePushed](Queue<int>* q) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs - 10));
            q->push(valuePushed);
        },
        &q);

    queueReaderThread.join();
    queueWriterThread.join();
}

TEST(Queue, popFromEmptyQueueBeingBlocked) {
    Queue<int> q(10);
    int valuePushed = 0xC0DE;
    int timeoutMs = 100;

    std::thread queueReaderThread(
        [timeoutMs, valuePushed](Queue<int>* q) {
            std::future<int> futureResult = std::async(std::launch::async, [q, timeoutMs, valuePushed]() { return q->pop(); });
            auto status = futureResult.wait_for(std::chrono::milliseconds(timeoutMs));
            ASSERT_EQ(status, std::future_status::timeout);
            ASSERT_EQ(futureResult.get(), valuePushed);
        },
        &q);

    std::thread queueWriterThread(
        [timeoutMs, valuePushed](Queue<int>* q) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs + 10));
            q->push(valuePushed);
        },
        &q);

    queueReaderThread.join();
    queueWriterThread.join();
}

TEST(Queue, popWithTimeoutFromEmptyQueueBeingUnblockedAfterPushedSomeElement) {
    Queue<int> q(10);
    int valuePushed = 0xC0DE;
    int timeoutMs = 100;

    std::thread queueReaderThread(
        [timeoutMs, valuePushed](Queue<int>* q) {
            std::future<int> futureResult = std::async(std::launch::async, [q, timeoutMs, valuePushed]() { return q->pop(timeoutMs); });
            auto status = futureResult.wait_for(std::chrono::milliseconds(timeoutMs));
            ASSERT_EQ(status, std::future_status::ready);
            ASSERT_EQ(futureResult.get(), valuePushed);
        },
        &q);

    std::thread queueWriterThread(
        [timeoutMs, valuePushed](Queue<int>* q) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs - 10));
            q->push(valuePushed);
        },
        &q);

    queueReaderThread.join();
    queueWriterThread.join();
}

TEST(Queue, popWithTimeoutFromEmptyQueueRaisingAnExceptionDueTimeout) {
    Queue<int> q(10);
    int timeoutMs = 100;

    ASSERT_THROW(q.pop(timeoutMs), QueueTimeoutException);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}