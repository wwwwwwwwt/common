/*
 * @Author: zzzzztw
 * @Date: 2023-02-24 11:28:22
 * @LastEditors: Do not edit
 * @LastEditTime: 2023-03-18 11:10:23
 * @FilePath: /cpptest/tinythreadpool-c++11/threadpool.h
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <future>


//多线程安全队列
template<typename T>
class SafeQueue{
public:

    SafeQueue() = default;
    SafeQueue(SafeQueue &&s) =  default;
    SafeQueue& operator= (SafeQueue&& s) = default;

    void enqueue(const T& t){
        std::unique_lock<std::mutex>locker(mtx_);
        que_.emplace(t);
    }

    void enqueue(T &&t){
        std::unique_lock<std::mutex>locker(mtx_);
        que_.emplace(t);
    }


    bool dequeue(T& t){//队列为空也能返回结果
        std::unique_lock<std::mutex>locker(mtx_);

        if(que_.empty())return false;

        t = std::move(que_.front());
        que_.pop();
        return true;
    }

    void clean(){//用于退出
        std::unique_lock<std::mutex>locker(mtx_);
        while(!que_.empty())que_.pop();
    }

    bool empty(){
        std::unique_lock<std::mutex>locker(mtx_);
        return que_.empty();
    }

    int size(){
        std::unique_lock<std::mutex>locker(mtx_);
        return que_.size();
    }

    ~SafeQueue(){
        clean();
    }

private:
    std::mutex mtx_;
    std::queue<T>que_;
    
};

class ThreadPool{

private:
    SafeQueue<std::function<void()>>queue_;
    std::vector<std::thread>threads_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool>stop_;

private:
    class ThreadWorker{
    
    public:
        ThreadWorker(ThreadPool* pool, int id):pool_(pool),id_(id){};
        void operator()(){
            printf("thread [%d] : enter.\n",id_);
            std::function<void()>func;
            bool hastask = false;

            while(!pool_->stop_){
                {
                    std::unique_lock<std::mutex>locker(pool_->mutex_);
                    printf("threadpool [%d]: waiting task\n",id_);
                    pool_->cv_.wait(locker,[&]{
                        if(pool_->stop_)return true;
                        return !pool_->queue_.empty();
                    });

                    hastask = pool_->queue_.dequeue(func);
                }

                if(hastask){
                    printf("threadpool [%d]: is working\n",id_);
                    func();
                    printf("threadpool [%d] work done.\n",id_);
                }
            }

            printf("threadpool [%d] exit.\n",id_);

        }
    private:
        ThreadPool* pool_;
        int id_;
    };
public:
    
    explicit ThreadPool(const int n_threads = 4):threads_(std::vector<std::thread>(n_threads)),stop_(false){};
    ThreadPool(const ThreadPool& t) = delete;
    ThreadPool(ThreadPool &&t) = delete;
    ThreadPool& operator=(const ThreadPool& t) = delete;
    ThreadPool& operator=(ThreadPool&& t) = delete;
    ~ThreadPool(){
        stop();
        std::cout<<"done\n"<<std::endl;
    }
    void init(){
        for(size_t i = 0;i < threads_.size();i++){
            threads_.at(i) = std::thread(ThreadWorker(this,i));
        }
    }
    void stop(){
        stop_ = true;
        cv_.notify_all();

        for(size_t i = 0;i < threads_.size();i++){
            if(threads_[i].joinable()){
                std::cout<<"thread ["<<i<<"] join\n";
                threads_[i].join();
            }
        }
    }

    template<typename F, typename ...Args>
    auto AddTask(F&&f, Args &&...args)->std::future<decltype(f(args...))>{
        using ret = decltype(f(args...));
        std::function<ret()>func = std::bind(std::forward<F>(f),std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<ret()>>(func);

        std::function<void()> wrapper_func = [task_ptr](){
            (*task_ptr)();
        };
        queue_.enqueue(wrapper_func);
        cv_.notify_one();
        return task_ptr->get_future();
    }
    
};

#endif