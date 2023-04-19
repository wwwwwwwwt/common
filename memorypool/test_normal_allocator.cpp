/*
 * @Author: zzzzztw
 * @Date: 2023-04-19 14:40:08
 * @LastEditors: Do not edit
 * @LastEditTime: 2023-04-19 14:40:17
 * @FilePath: /myLearning/memorypool/test_normal_allocator.cpp
 */

#include <thread>               // std::thread
#include <unordered_set>        // std::unordered_set
#include "container_op_test.h"  // test_container

using namespace std;

using TestType = unordered_set<int>;

int main()
{
    // Linux shows a performance difference, depending on whether there
    // have been more than one thread.
    thread t{[] {}};
    t.join();

    TestType s;
    test_container(s);
}