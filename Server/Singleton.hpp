/*
 * @Author: LeiJiulong
 * @Date: 2024-11-27 20:27:41
 * @LastEditors: LeiJiulong && lei15557570906@outlook.com
 * @LastEditTime: 2024-11-27 20:37:42
 * @Description:
 */
#pragma once
#include <memory>
#include <mutex>
#include <iostream>

template <typename T>
class Singleton
{
protected:
    Singleton() = default;
    Singleton(const Singleton<T> &) = delete;
    Singleton &operator=(const Singleton<T> &) = delete;

    static std::shared_ptr<T> instance_;

public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag Singleton_flag;
        std::call_once(Singleton_flag,
                       [this]
                       {
                           instance_ = std::shared_ptr<T>(new T);
                       });
        return instance_;
    }

    void printAddress()
    {
        std::cout << instance_.get() << std::endl;
    }

    ~Singleton()
    {
        std::cout << "this singleton is destruct " << std::endl;
    }
};

template <typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;