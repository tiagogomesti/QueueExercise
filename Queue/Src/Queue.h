#include <chrono>  // std::chrono::seconds
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

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
        std::cout << "Hello from Queue" << std::endl;
    }

    void push(T element) {
        std::lock_guard<std::mutex> guard(queueMutex);
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
        std::unique_lock uLock(queueMutex);

        switch (queueState) {
        case QueueState::EMPTY:
            uLock.unlock();
            conditionVariable.wait(uLock, [this] { return this->queueState != QueueState::EMPTY; });
            uLock.lock();
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

        uLock.unlock();
        return t;
    }

    T pop(int milisecondsTimeout) {
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        return T{};
    }

    int count() {
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