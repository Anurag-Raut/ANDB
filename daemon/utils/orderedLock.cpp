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
    uint64_t txLock=0;

   public:
    bool wasPrevTransactionId=false;
    void lock(uint64_t current_transaction_id, vector<uint64_t> waitQueue) {
        if(isLocked==true && txLock==current_transaction_id){
            return;
        }
        std::unique_lock<std::mutex> lock(mtx);
        waitQueue.push_back(current_transaction_id);
        cv.wait(lock, [&] { return !isLocked && (waitQueue.empty() || waitQueue.front() == current_transaction_id); });
        waitQueue.erase(waitQueue.begin());
        isLocked = true;
        txLock=current_transaction_id;
    }
    void unlock(bool isCommit) {
        std::unique_lock<std::mutex> lock(mtx);
        isLocked = false;
        wasPrevTransactionId=isCommit;
        lock.unlock();
        cv.notify_all();
    }
};