#include <iostream>
#include "readwirtelock.h"
using namespace std;


void reader(WriteReadLock& lock, int i){
    lock.lock_read();
    std::cout<<"reader "<<i<<" is enter"<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout<<"reader "<<i<<" is out"<<std::endl;
    lock.unlock_read();
}

void Writer(WriteReadLock&lock, int i){
    lock.lock_write();
    std::cout<<"writer "<<i << " is enter"<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout<<"writer "<<i<<" is out"<<std::endl;
    lock.unlock_write();
}

int main(){
    WriteReadLock lock;

    for(int i = 0; i < 10; i++){
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        if(i == 3 || i == 5){
            std::thread t1(&Writer, std::ref(lock), i);
            t1.detach();
        }
        std::thread t2(&reader, std::ref(lock), i);
        t2.detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;

}