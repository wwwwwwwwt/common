# 一个简单Vector
## 整体思路
1. 使用两个变量m_size和m_capacity记录当前使用的容量和最大容量
2. 当前使用大小到达最大容量时，使用ReAlloc()进行扩容
3. 利用模板函数设计了简单接口：PushBack(),EmplaceBack(),PopBack(),Clear()

## 细节实现
1. 在ReAlloc中，当模板参数类似为结构体，存在指针指向一片内存空间时，当popback()调用时会调用该结构体的析构函数，
然后在Vector的析构函数中还会调用一次，该内存会被delete两次。
2. 解决的办法是使用 ::operator new/delete 将 new/delete 构造函数+分配内存两步分开 ::operator new + 定位new 代替new
3. clear()调用结构体析构函数 + ::operator delete 删除内存 代替delete 
