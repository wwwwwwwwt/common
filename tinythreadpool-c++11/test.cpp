/*
 * @Author: zzzzztw
 * @Date: 2023-02-25 12:53:15
 * @LastEditors: Do not edit
 * @LastEditTime: 2023-02-25 13:34:17
 * @FilePath: /tinythreadpool-c++11/test.cpp
 */
#include <iostream>
#include "./threadpool.h"

int main(){
    ThreadPool pools(4);
    pools.init();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    printf("---------------------------------------\n");

    std::vector<std::future<int>>vec;
    
    for(int i = 0;i<20;i++){
        printf("addtask [%d]\n",i);
        vec.push_back(pools.AddTask([i](){
            std::this_thread::sleep_for(std::chrono::microseconds(rand()%500+100));
            printf("task [%d] done\n",i);
            return i;
        }));
    }
    printf("---------------------------------------\n");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    for(size_t i = 0;i<vec.size();i++){
        printf("result: %d \n",vec[i].get());
    }

    printf("---------------------------------------\n");

    

    return 0;

}