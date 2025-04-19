#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

class QueueTimeoutException : public std::runtime_error {
public:
    QueueTimeoutException(const std::string& message)
        : std::runtime_error(message) {
    }
};

template<typename T>
class Queue {
public:
    Queue(uint32_t size)
        : queueSize(size)
        , queue{new T*[size]}
        , header(0)
        , tail(0)
        , isFull(false)
        , queueState(QueueState::EMPTY) {
    }

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

    T pop(int milisecondsTimeout) {
        T t;
        std::unique_lock<std::mutex> lock(queueMutex);

        switch (queueState) {
        case QueueState::EMPTY:
            if (conditionVariable.wait_for(lock, std::chrono::milliseconds(milisecondsTimeout), [this] {
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

    int count() {
        std::lock_guard<std::mutex> guard(queueMutex);
        return header > tail ? header - tail : header + queueSize - tail;
    }

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