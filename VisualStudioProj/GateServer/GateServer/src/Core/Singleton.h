#pragma once

#include <memory>
#include <mutex>

template<typename T>
class Singleton
{
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;                // ȡ�����ƹ��ܣ����ƺ�����
    Singleton& operator= (const Singleton<T>&) = delete;    // ��ֹ�� Singleton ���и�ֵ����
protected:
    static std::shared_ptr<T> s_Instance;

public:
    ~Singleton() { JC_CORE_TRACE("This singleton is destructed! \n") }

    void PrintAddress() { JC_CORE_TRACE("{} \n", s_Instance.get()) }

    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag s_Flag;
        std::call_once(s_Flag, [&]() { s_Instance = std::shared_ptr<T>(new T); });

        return s_Instance;
    };
};

template<typename T>
std::shared_ptr<T> Singleton<T>::s_Instance = nullptr;
