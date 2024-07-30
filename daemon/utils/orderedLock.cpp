#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include<iostream>

using namespace std;

class OrderedLock {
    mutex mtx;
    condition_variable cv;
    bool isLocked = false;

   public:
    void lock(uint64_t current_transaction_id, vector<uint64_t> waitQueue) {
        std::unique_lock<std::mutex> lock(mtx);
        waitQueue.push_back(current_transaction_id);
        cv.wait(lock, [&] { return !isLocked && (waitQueue.empty() || waitQueue.front() == current_transaction_id); });
        waitQueue.erase(waitQueue.begin());
        cout << "LOCK ACQIORED BY: " << current_transaction_id << endl;
        isLocked = true;
    }
    void unlock() {
        std::unique_lock<std::mutex> lock(mtx);
        isLocked = false;
        cout << "LOCK RELEASE: " << endl;

        lock.unlock();
        cv.notify_all();
    }
};