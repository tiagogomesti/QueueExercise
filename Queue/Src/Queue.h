/**
 * @file Queue.h
 * @author Tiago Gomes (tiagogomes.ti@gmail.com)
 * @brief Generic data queue class
 * @version 0.1
 * @date 2025-04-21
 *
 */

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

/**
 * @brief Class used to raise an exception when a pop with a timeout
 * is used and this timeout is exceeded.
 *
 */
class QueueTimeoutException : public std::runtime_error {
public:
    QueueTimeoutException(const std::string& message)
        : std::runtime_error(message) {
    }
};

/**
 * @brief Class responsible for implementing a queue for data of type \p T.
 *
 * @tparam T The data type of the elements in the queue.
 */
template<typename T>
class Queue {
public:
    /**
     * @brief Constructs a new Queue object for data of type \p T.
     *
     * @param[in] size The size of the queue.
     */
    Queue(uint32_t size)
        : queueSize(size)
        , queue{new T*[size]}
        , header(0)
        , tail(0)
        , isFull(false)
        , queueState(QueueState::EMPTY) {
    }

    /**
     * @brief Destroys the Queue object, deleting the allocated memory.
     *
     */
    ~Queue() {
        std::lock_guard<std::mutex> guard(queueMutex);

        if (queueState != QueueState::EMPTY) {
            do {
                delete queue[header];
                header = (header + 1) % queueSize;
            } while (header != tail);
        }

        queueState = QueueState::EMPTY;
    }

    /**
     * @brief Pushes a new \p element into the queue. If the queue is full, the \p element overwrites
     * the oldest element.
     *
     * @param[in] element The element of type T to be pushed into the queue.
     */
    void push(T element) {
        std::unique_lock<std::mutex> lock(queueMutex);
        switch (queueState) {
        case QueueState::EMPTY:
            queue[header] = new T(element);
            header = (header + 1) % queueSize;
            queueState = (header == tail) ? QueueState::FULL : QueueState::NOT_FULL_EMPTY;
            conditionVariable.notify_one();
            break;

        case QueueState::NOT_FULL_EMPTY:
            queue[header] = new T(element);
            header = (header + 1) % queueSize;
            queueState = (header == tail) ? QueueState::FULL : QueueState::NOT_FULL_EMPTY;
            break;

        case QueueState::FULL:
            *queue[header] = element;
            tail = header = (header + 1) % queueSize;
            break;
        }
    }

    /**
     * @brief Returns the oldest element in the queue. If the queue is empty, this method will
     * block the current thread execution until a new element is pushed.
     *
     * @return T The oldest element in the queue.
     */
    T pop() {
        T t;
        std::unique_lock<std::mutex> lock(queueMutex);

        switch (queueState) {
        case QueueState::EMPTY:
            conditionVariable.wait(lock, [this] { return this->queueState != QueueState::EMPTY; });
            t = *queue[tail];
            delete queue[tail];
            tail = (tail + 1) % queueSize;
            queueState = (header == tail) ? QueueState::EMPTY : QueueState::NOT_FULL_EMPTY;
            break;

        case QueueState::NOT_FULL_EMPTY:
        case QueueState::FULL:
            t = *queue[tail];
            delete queue[tail];
            tail = (tail + 1) % queueSize;
            queueState = (header == tail) ? QueueState::EMPTY : QueueState::NOT_FULL_EMPTY;
            break;
        }

        return t;
    }

    /**
     * @brief Returns the oldest element in the queue. If the queue is empty, it will
     * block the current thread execution until a new element is pushed. If
     * the waiting time exceeds \p millisecondsTimeout, a QueueTimeoutException
     * exception will be thrown.
     *
     * @param[in] millisecondsTimeout The timeout in milliseconds.
     * @return T The oldest element in the queue.
     * @throws QueueTimeoutException If the timeout is exceeded before an element is available.
     */
    T pop(int millisecondsTimeout) {
        T t;
        std::unique_lock<std::mutex> lock(queueMutex);

        switch (queueState) {
        case QueueState::EMPTY:
            if (conditionVariable.wait_for(lock, std::chrono::milliseconds(millisecondsTimeout), [this] {
                    return this->queueState != QueueState::EMPTY;
                })) {
                t = *queue[tail];
                delete queue[tail];
                tail = (tail + 1) % queueSize;
                queueState = (header == tail) ? QueueState::EMPTY : QueueState::NOT_FULL_EMPTY;
            } else {
                throw QueueTimeoutException("Pop timeout");
            }
            break;

        case QueueState::NOT_FULL_EMPTY:
        case QueueState::FULL:
            t = *queue[tail];
            delete queue[tail];
            tail = (tail + 1) % queueSize;
            queueState = (header == tail) ? QueueState::EMPTY : QueueState::NOT_FULL_EMPTY;
            break;
        }

        return t;
    }

    /**
     * @brief Returns the current number of elements in the queue.
     *
     * @return int The number of elements.
     */
    int count() {
        std::lock_guard<std::mutex> guard(queueMutex);

        int nElements = 0;

        switch (queueState) {
        case QueueState::EMPTY:
            nElements = 0;
            break;

        case QueueState::NOT_FULL_EMPTY:
            nElements = header >= tail ? header - tail : header + queueSize - tail;
            break;

        case QueueState::FULL:
            nElements = queueSize;
            break;
        }

        return nElements;
    }

    /**
     * @brief Returns the size of the queue.
     *
     * @return int The queue size.
     */
    int size() {
        return queueSize;
    }

private:
    enum class QueueState { EMPTY, NOT_FULL_EMPTY, FULL };

    T** queue;
    const uint32_t queueSize;
    uint32_t header;
    uint32_t tail;
    bool isFull;
    QueueState queueState;
    std::mutex queueMutex;
    std::condition_variable conditionVariable;
};