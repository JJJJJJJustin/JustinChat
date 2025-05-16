#ifndef SINGLETON_H
#define SINGLETON_H

#include "global.h"

template<typename T>
class Singleton
{
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;                // 取消复制功能（复制函数）
    Singleton& operator= (const Singleton<T>&) = delete;    // 禁止对 Singleton 进行赋值操作
protected:
    static std::shared_ptr<T> s_Instance;

public:
    ~Singleton() { std::cout << "This singleton is destructed!" << std::endl; }

    void PrintAddress(){ std::cout << s_Instance.get() << std::endl; }

    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag s_Flag;
        std::call_once(s_Flag, [&](){ s_Instance = std::shared_ptr<T>(new T); });

        return s_Instance;
    };
};

template<typename T>
std::shared_ptr<T> Singleton<T>::s_Instance = nullptr;

#endif // SINGLETON_H
