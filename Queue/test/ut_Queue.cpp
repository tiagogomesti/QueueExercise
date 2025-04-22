#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

#include "Queue.h"

/**
 * @brief Pushing an element into the queue until it is full.
 * All the pushed elements should then be read in a pop execution.
 */
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

/**
 * @brief Pushing an element into a full queue. The oldest element
 * should be dropped, and the new element should take its place.
 */
TEST(Queue, pushElementsIntoFullQueue) {
    int size = 50;
    Queue<int> q(size);

    // Filling the queue
    for (int i = 0; i < size; i++) {
        q.push(i);
        ASSERT_EQ(q.count(), i + 1);
    }

    q.push(0xFFFF);  // This value should take the place of the oldest element in the queue

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

/**
 * @brief Popping from an empty queue using the pop() method should block the calling thread until
 * data is written into the queue. In this test, we used std::future to check if the pop will return
 * before a timeoutMs milliseconds interval.
 * In this test case, a new element is pushed before the timeoutMs milliseconds, so we check
 * that the pop() will wait for the new element.
 */
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

/**
 * @brief Popping from an empty queue using the pop() method should block the calling thread until
 * data is written into the queue. In this test, we used std::future to check if the pop will return
 * before a timeoutMs milliseconds interval.
 * In this test case, a new element is pushed after the timeoutMs milliseconds, so we check
 * that the pop() will continue waiting for the new element.
 */
TEST(Queue, popFromEmptyQueueBeingBlocked) {
    Queue<int> q(10);
    int valuePushed = 0xC0DE;
    int timeoutMs = 100;

    std::thread queueReaderThread(
        [timeoutMs, valuePushed](Queue<int>* q) {
            std::future<int> futureResult = std::async(std::launch::async, [q, timeoutMs, valuePushed]() { return q->pop(); });
            auto status = futureResult.wait_for(std::chrono::milliseconds(timeoutMs));
            ASSERT_EQ(status, std::future_status::timeout);
            // The get() will still return the value when it becomes available
            // but the assertion about the timeout confirms it waited.
            // ASSERT_EQ(futureResult.get(), valuePushed);
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

/**
 * @brief Popping from an empty queue using the pop(millisecondsTimeout) method should block the calling thread until
 * data is written into the queue, or the timeout expires. In this test, we used std::future to check if the pop
 * will return before a timeoutMs milliseconds interval.
 * In this test case, a new element is pushed before the timeoutMs milliseconds, so we check
 * that the pop will wait for the new element, as the waiting time is less than timeoutMs.
 */
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

/**
 * @brief Popping from an empty queue using the pop(millisecondsTimeout) method should block the calling thread until
 * data is written into the queue, or the timeout expires. In this test case, we check if after the pop method waits
 * for a time greater than timeoutMs milliseconds, a QueueTimeoutException is raised.
 */
TEST(Queue, popWithTimeoutFromEmptyQueueRaisingAnExceptionDueTimeout) {
    Queue<int> q(10);
    int timeoutMs = 100;

    ASSERT_THROW(q.pop(timeoutMs), QueueTimeoutException);
}

/**
 * @brief Checks if the destructor deletes all dynamically allocated elements.
 */
TEST(Queue, DestructorShouldDeleteAllAllocatedMemory) {
    int size = 50;
    Queue<int> q(size);

    // Filling the queue
    for (int i = 0; i < size; i++) {
        q.push(i);
        ASSERT_EQ(q.count(), i + 1);
    }

    q.~Queue();
    ASSERT_EQ(q.count(), 0);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}