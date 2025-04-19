#include <chrono>  // std::chrono::seconds
#include <iostream>
#include <thread>

#include "Queue.h"

using namespace std;

#define N_ELEMENTS 50

void queueReaderThreadFunc(Queue<int>* queue) {
    // cout << "queueReaderThreadFun initialized" << endl;

    for (int i = 0; i < 50; i++) {
        cout << "Starting pop: " << i << endl;
        int temp = queue->pop(1010);
        cout << " pop: " << i << " finished. Value poped  = " << temp << endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void queueWriterThreadFunc(Queue<int>* queue) {
    // cout << "queueWriterThreadFun initialized" << endl;

    for (int i = 0; i < 50; i++) {
        int temp = i + 100;
        cout << "Starting push: " << i << endl;
        queue->push(temp);
        cout << "push: " << i << " finished. Value pushed = " << temp << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int, char**) {
    cout << "Hello, from Queue!" << endl;
    Queue<int> q(10);

    thread queueReaderThread(queueReaderThreadFunc, &q);
    thread queueWriterThread(queueWriterThreadFunc, &q);
    queueReaderThread.join();
    queueWriterThread.join();
}
