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
        cout<<"TRNASCTION ID:"<<current_transaction_id<<endl;
        waitQueue.push_back(current_transaction_id);
        cv.wait(lock, [&] { return !isLocked && (waitQueue.empty() || waitQueue.front() == current_transaction_id); });
        cout<<"TRNASCTION ID AFTE CV :"<<waitQueue.front()<<endl;
        waitQueue.erase(waitQueue.begin());
        cout << "LOCK ACQIORED BY: " << current_transaction_id << endl;
        isLocked = true;
        txLock=current_transaction_id;
    }
    void unlock(bool isCommit) {
        cout<<"UNLOCKING: "<<isCommit<<endl;
        std::unique_lock<std::mutex> lock(mtx);
        isLocked = false;
        cout << "LOCK RELEASE: " << endl;
        wasPrevTransactionId=isCommit;
        lock.unlock();
        cv.notify_all();
    }
};