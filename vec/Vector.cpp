/*
 * @Author: zzzzztw
 * @Date: 2023-02-02 20:08:23
 * @LastEditors: Do not edit
 * @LastEditTime: 2023-02-25 16:40:23
 * @FilePath: /cpptest/Vec/Vector.cpp
 */
#include<iostream>
#include "Vector.h"


struct Vector3{
    float x = 0.0f, y = 0.0f,z = 0.0f;

    Vector3(){}
    Vector3(float val):x(val),y(val),z(val){
        
    }
    Vector3(float x,float y, float z):x(x),y(y),z(z){}
    Vector3(const Vector3& other):x(other.x),y(other.y),z(other.z)
    {
        std::cout<<"copy"<<std::endl;
    }
    Vector3(Vector3&& other):x(other.x),y(other.y),z(other.z)
    {
        std::cout<<"move"<<std::endl;
    }
    ~Vector3(){
        std::cout<<"destory"<<std::endl;
    }

    Vector3 &operator=(const Vector3& other){
        std::cout<<"copy"<<std::endl;
        x  = other.x;
        y  = other.y;
        z  = other.z;
        return *this;
    }
    Vector3 &operator=(Vector3&& other){
        std::cout<<"move"<<std::endl;
        x  = other.x;
        y  = other.y;
        z  = other.z;
        return *this;
    }
};

template<typename T>
void Print(const Vector<T> &ve){
    for(size_t i = 0;i<ve.Size();i++){
        std::cout<<ve[i]<<std::endl;
    }
    std::cout<<"--------------------------"<<std::endl;
}
template<>//模板具体化 特例
void Print(const Vector<Vector3> &ve){
    for(size_t i = 0;i<ve.Size();i++){
        std::cout<<ve[i].x<<ve[i].y<<ve[i].z<<std::endl;
    }
    std::cout<<"--------------------------"<<std::endl;
}

int main(){
    // Vector<std::string>V;
    //V.PushBack("ztw");
    //V.PushBack("ABC");
    //Print(V);

    Vector<Vector3>vect;
  /*  vect.PushBack(Vector3(1.0f));
    vect.PushBack(Vector3(2,3,4));
    vect.PushBack(Vector3())*/
    vect.EmplaceBack(1.0,2.0,3.0);
    vect.EmplaceBack(2.0,3.0,4.0);
    vect.EmplaceBack(5.0,6.0,7.0);
    vect.PopBack();    
    Print(vect);


    std::cout<<vect.Size()<<std::endl;

    return 0;

}