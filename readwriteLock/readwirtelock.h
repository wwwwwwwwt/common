/*
 * @Author: zzzzztw
 * @Date: 2023-06-27 17:05:48
 * @LastEditors: Do not edit
 * @LastEditTime: 2023-06-27 17:33:53
 * @FilePath: /myLearning/readwriteLock/readwirtelock.h
 */
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

class WriteReadLock{

public:
    WriteReadLock() = default;
    ~WriteReadLock() = default;
    WriteReadLock(const WriteReadLock&) = delete;
    WriteReadLock& operator=(const WriteReadLock&) = delete;

public:
    void lock_read(){
        std::unique_lock<std::mutex>locker(mtx_);
        cv_read.wait(locker, [=](){
            return write_cnt == 0;
        });
        ++read_cnt;
    }

    void lock_write(){
        std::unique_lock<std::mutex>locker(mtx_);
        ++write_cnt;
        cv_write.wait(locker, [=](){
            return read_cnt == 0 && !inwriting;
        });
        inwriting = true;
    }

    void unlock_read(){
        std::unique_lock<std::mutex>locker(mtx_);
        if(--read_cnt == 0 && write_cnt > 0){
            cv_write.notify_one();
        }
    }

    void unlock_write(){
        std::unique_lock<std::mutex>locker(mtx_);
        inwriting = false;
        if(--write_cnt == 0){
            cv_read.notify_all();
        }else{
            cv_write.notify_one();
        }
        
    }

private:
    volatile size_t read_cnt {0};
    volatile size_t write_cnt {0};
    volatile bool inwriting{false};
    std::mutex mtx_;
    std::condition_variable cv_write;
    std::condition_variable cv_read;
};

