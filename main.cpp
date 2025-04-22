#include <chrono>
#include <iostream>
#include <thread>

#include "Queue.h"

using namespace std;

#define QUEUE_SIZE 50
#define MIN_WAIT_TIME_SEC 1
#define MAX_WAIT_TIME_SEC 5
#define NUM_ELEMENTS_PUSHED 100
#define NUM_ELEMENTS_POPPED 100

void queueReaderThreadFunc(Queue<int>* queue) {
    cout << "queueReaderThreadFun initialized" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int i = 0; i < NUM_ELEMENTS_POPPED; i++) {
        int randWaitingTime = MIN_WAIT_TIME_SEC + (rand() % (MAX_WAIT_TIME_SEC - MIN_WAIT_TIME_SEC + 1));
        std::this_thread::sleep_for(std::chrono::seconds(randWaitingTime));
        int temp = queue->pop();
        cout << "#Reader thread: pop() --> " << temp << endl;
    }
}

void queueWriterThreadFunc(Queue<int>* queue) {
    cout << "queueWriterThreadFunc initialized" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int i = 0; i < NUM_ELEMENTS_POPPED; i++) {
        int randWaitingTime = MIN_WAIT_TIME_SEC + (rand() % (MAX_WAIT_TIME_SEC - MIN_WAIT_TIME_SEC + 1));
        std::this_thread::sleep_for(std::chrono::seconds(randWaitingTime));
        queue->push(i);
        cout << "#Writer thread: push() --> " << i << endl;
    }
}

int main(int, char**) {
    Queue<int> q(QUEUE_SIZE);

    srand(time(0));
    thread queueReaderThread(queueReaderThreadFunc, &q);
    thread queueWriterThread(queueWriterThreadFunc, &q);
    queueReaderThread.join();

    queueWriterThread.join();
}
