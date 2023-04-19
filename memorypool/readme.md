<!--
 * @Author: zzzzztw
 * @Date: 2023-04-19 13:27:41
 * @LastEditors: Do not edit
 * @LastEditTime: 2023-04-19 16:35:44
 * @FilePath: /myLearning/memorypool/readme.md
-->
# 快速分配和释放内存：内存池

使用内存池理由：
1. 希望减少内存分配和释放的开销，更快的分配和释放
2. 希望减少内存分配和释放的开销，更少的总体内存使用  
   
内存池以下情况慎重使用  
1. 内存池可能导致操作系统更难回收你不需要的内存
2. 通用容器分配器已经做够快了
3. 内存池应该在单线程或者线程本地使用，否则性能可能会更差，不如通用内存分配器好

本内存池实现做法：  
1. 利用数组开辟一大块内存
2. 某个特定类型的对象的内存分配请求都到某一个内存池里分配和释放
3. 期间不返回内存给内存分配器，返回内存更容易导致内存碎片，也会有更简单的代码实现

## 自定义内存池

1. 定义内存池结构：

* 定义内存池存储的数据对象的Node()包含数据段和一个指针，指向他的下一个内存块

```cpp
 union node{
     T data;
     node* next;
     node(){};
     ~node(){};
     node(const node&) = delete;
     node& operator=(const node&) = delete;
 };
```

* 定义内存块，内存块就是节点的数组和指向下一个内存块的指针，来把内存块串成一个链表

```cpp
 private:
     memory_chunk* next_chunk_{};
     std::array<node, memory_chunk_size<T>>storage_;

```
* 三个成员函数：用于初始化的构造函数，用于得到当前空闲节点函数（用于分配内存时得到空闲内存块），指向下一块内存池的函数（用于从头开始析构所有内存池）

```cpp
memory_chunk(memory_chunk* next);
node* get_free_nodes(){return storage_.data();};
memory_chunk* get_next_chunk() const{return next_chunk_;}

// 这里内存池的链表类似头插法，next都指向已经存在的上一个内存池
template <typename T>
memory_chunk<T>::memory_chunk(memory_chunk * next):next_chunk_(next)
{
 for(std::size_t i = 0; i< storage_.size()-1; i++){
     storage_[i].next = &storage_[i+1];
 }
 storage_[storage_.size() - 1].next = nullptr;
}

```

2. 定义内存池

* 两个成员变量：free_list当前内存池的空闲节点，chunk_list当前内存池（也是所有内存池的联编）
* 三个重要成员函数，析构函数（用于销毁所有存在的内存池），分配器allocate（用于找到空闲的内存块），释放器deallocate（将传入的内存指针放回空闲链表，可以重复使用）
  
```cpp

/*析构函数，根据内存池链表，挨个析构链表指针*/
template <typename T>
memory_pool<T>::~memory_pool(){
    while(chunk_list_){
        memory_chunk<T>* t = chunk_list_;
        chunk_list_ = chunk_list_->get_next_chunk();
        delete t;
    }
}

/*
    1. 如果没有空闲节点了，就新开一个内存池
    2. 将空闲节点中的数据块（内存块）传出
    3. 空闲链表移动到下一个空闲位置
*/
template <typename T>
T* memory_pool<T>::allocate(){

    if(free_list_ == nullptr){
        chunk_list_ = new memory_chunk<T>(chunk_list_);
        free_list_ = chunk_list_->get_free_nodes();
    }
    T* res = &free_list_->data;
    assert(reinterpret_cast<std::uintptr_t>(res) % sizeof(T*) == 0);
    free_list_ = free_list_->next;
    return res;
}


/*把传进来的结点指针重新放回空闲链表*/
template <typename T>
void memory_pool<T>::deallocate(T* ptr){
    assert(reinterpret_cast<std::uintptr_t>(ptr) % sizeof(T*) == 0);
    auto free_item = reinterpret_cast<node*>(ptr);
    free_item->next = free_list_;
    free_list_ = free_item;
}


```


## 应用1：特定类对象的分配和释放

1. 定义出内存池，然后在类内重写new和delete函数即可,类似实现如下：

```cpp

class Obj {
public:
    Obj() {}
    ~Obj() {}
    void* operator new(size_t size);
    void operator delete(void* ptr) noexcept;
};

memory_pool<Obj> obj_pool; // 定义内存池

void* Obj::operator new(size_t size)
{
    assert(size == sizeof(Obj));
    (void)size;
    return obj_pool.allocate();
}

void Obj::operator delete(void* ptr) noexcept
{
    obj_pool.deallocate(static_cast<Obj*>(ptr));
}


```

2. 测试代码见 test_new_delete_obj.cpp， 测试开启O2优化。根据测试发现，linux在是否开启多线程的条件下，性能有较大差异

```
1. 开启多线程：
 77 cycles for each allocation and deallocations on normal Obj
  7 cycles for each allocation and deallocations on pooled Obj
2. 关闭多线程：
 46 cycles for each allocation and deallocations on normal Obj
  8 cycles for each allocation and deallocations on pooled Obj

```
结果：在开启多线程时，性能提升约10倍，关闭多线程，性能提升约6倍

## 应用2：用于对单个对象的内存分配和释放使用线程内存池，适合list，map， set

1. 需要重写分配器，特别注意，构造函数中，需要先构造出内存池，使内存池的生命周期长于分配器，当分配单个对象内存时使用自定义内存分配器，多个时使用默认的，例子如下：

```cpp

template <typename T>
memory_pool<T>& get_memory_pool()
{
    thread_local memory_pool<T> pool;
    return pool;
}

template <typename T, typename Base = std::allocator<T>>
struct pooled_allocator : private Base {
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_type = T;

    using propagate_on_container_copy_assignment =
        typename std::allocator_traits<
            Base>::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment =
        typename std::allocator_traits<
            Base>::propagate_on_container_move_assignment;
    using propagate_on_container_swap =
        typename std::allocator_traits<Base>::propagate_on_container_swap;
    using is_always_equal =
        typename std::allocator_traits<Base>::is_always_equal;

    template <class U>
    struct rebind {
        using other = pooled_allocator<U>;
    };

    pooled_allocator()
    {
        // Ensure the early creation of the memory pool.  Without this
        // call, a thread-local object may have lifetime issues using
        // this memory pool -- the memory pool may be destroyed earlier
        // than the object.  Also notice that a global/static object
        // generally cannot use this (thread-local) allocator safely.
        (void)get_memory_pool<T>();
    }

    template <class U>
    pooled_allocator(const pooled_allocator<U>&)
    {
    }

    T* allocate(std::size_t n)
    {
        if (n == 1) {
            return get_memory_pool<T>().allocate();
        } else {
            return Base::allocate(n);
        }
    }

    void deallocate(T* p, std::size_t n)
    {
        if (n == 1) {
            return get_memory_pool<T>().deallocate(p);
        } else {
            return Base::deallocate(p, n);
        }
    }
};



```

测试代码见test_pooled_allocator.cpp与 test_normal_allocator.cpp，使用unordered_map进行进行插入-删除-再插入的测试,提升约50%

```cpp
1. 使用默认分配器：
   It took 226 cycles by average to insert a number
   It took 132 cycles by average to erase  a number
   It took 113 cycles by average to insert a number again
2. 使用自定义分配器：
   It took 110 cycles by average to insert a number
   It took  45 cycles by average to erase  a number
   It took  47 cycles by average to insert a number again
```