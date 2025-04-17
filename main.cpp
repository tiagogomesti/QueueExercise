#include <iostream>
#include <thread>

#include "Queue.h"

using namespace std;

void queueReaderThreadFunc(void);
void queueWritterThreadFunc(void);


int main(int, char**) {
    cout << "Hello, from Queue!" << endl;
    Queue queue;

    thread queueReaderThread(queueReaderThreadFunc);
    thread queueWritterThread(queueWritterThreadFunc);
    queueReaderThread.join();
    queueWritterThread.join();
}

void queueReaderThreadFunc(void) {
    cout << "queueReaderThreadFun says: " << endl;
}

void queueWritterThreadFunc(void) {
    cout << "queueWritterThreadFun says: " << endl;
}