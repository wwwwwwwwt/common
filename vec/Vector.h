#ifndef MYVECTOR_H
#define MYVECTOR_H

#include <cassert>
#include <functional>
using size_t = unsigned long;

template<typename T>
class Vector
{
public:
    Vector(){
        //allocate 2 elememrs
        ReAlloc(2);
    }
    ~Vector(){
        Clear();
        ::operator delete(m_Data, m_Capacity * sizeof(T));
    }
    void PushBack(const T& value){
        if(m_Size >= m_Capacity){
            ReAlloc(m_Capacity *2 );
        }
        m_Data[m_Size++] = value;
    }

    void PushBack(T&& value){
        if(m_Size >= m_Capacity){
            ReAlloc(m_Capacity *2 );
        }
        m_Data[m_Size++] = std::move(value);
    }

    template<typename... Args>
    void EmplaceBack(Args&& ...args){
        
        if(m_Size>= m_Capacity){
            ReAlloc(m_Capacity * 2);
        }

       // m_Data[m_Size++] = T(std::forward<Args>(args)...);
        new(&m_Data[m_Size++])T(std::forward<Args>(args)...);


    }

    void PopBack(){

        if(m_Size>0){

            m_Size--;   
            m_Data[m_Size].~T();

        }
    }
    void Clear(){
        for(size_t i = 0;i<m_Size;i++){
            m_Data[i].~T();
        }
        m_Size = 0;
    }

    T& operator[](size_t idex){
        assert(idex < m_Size&&idex>=0);
        return m_Data[idex];
    }

    const T& operator[](size_t idex)const {
        assert(idex < m_Size&&idex>=0);
        return m_Data[idex];
    }

    size_t Size()const{return m_Size;}

private:

    void ReAlloc(size_t newCapacity)
    {
        //1. allocate a new block of memory
        //2. copy / move old memory to new block
        //3. delete

        T* newBlock = (T*)::operator new(newCapacity* sizeof(T));

        if(newCapacity < m_Size)
            m_Size = newCapacity;
        for(size_t i = 0;i < m_Size;i++){
           // newBlock[i] = std::move(m_Data[i]);
           new(&newBlock[i])T(std::move(m_Data[i]));
        }

        for(size_t i = 0;i<m_Size;i++){
            m_Data[i].~T();
        }
        ::operator delete(m_Data,m_Capacity * sizeof(T));
        m_Data = newBlock;
        m_Capacity = newCapacity;

    }

private:
    T * m_Data = nullptr;

    size_t  m_Size = 0;
    size_t  m_Capacity = 0;
};

#endif