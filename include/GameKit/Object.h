#pragma once

#include <algorithm>
#include <iostream>
#include <list>
#include <string>

#include <stddef.h>

#include "GameKit/LogHelper.h"

namespace games
{

template <typename DerivedT>
class IntrusivePtr
{
public:
    void increase_ref()
    {
        ref_count_++;
    }
    void decrease_ref()
    {
        ref_count_--;
    }

    size_t ref_count() const
    {
        return ref_count_;
    }

    void decrease_to_zero()
    {
        ref_count_ = 0;
    }

protected:
    int64_t ref_count_ = 0;
};

class ObjectBase : public IntrusivePtr<ObjectBase>
{
public:
    static size_t gMemoryStatistic;
    ObjectBase();
    virtual ~ObjectBase();
    // object is alive, die object should not draw on window and need to destory

    virtual void update(size_t tick) = 0;

    virtual void draw() = 0;

    virtual std::string name()
    {
        return "ObjectBase";
    }

    virtual void die()
    {
        alive_ = false;
    }

    bool alive()
    {
        return alive_ > 0;
    }

protected:
    int64_t alive_ = 1;
};

template <typename T>
class SharedObject
{
public:
    SharedObject(T* ptr = nullptr) : ptr_(ptr)
    {
        if (ptr_)
            ptr_->increase_ref();
    }

    SharedObject(const SharedObject<T>& other)
    {
        *this = other;
    }

    SharedObject(SharedObject<T>&& other)
    {
        *this = std::move(other);
    }

    SharedObject& operator=(const SharedObject<T>& other)
    {
        ptr_ = other.ptr_;
        ptr_->increase_ref();
        return *this;
    }

    SharedObject& operator=(SharedObject<T>&& other)
    {
        ptr_ = std::exchange(other.ptr_, ptr_);
        return *this;
    }

    ~SharedObject()
    {
        if (ptr_)
        {
            if (1 == ptr_->ref_count())
            {
                Log::info() << "destroy:" << typeid(*ptr_).name()
                            << " ref count:" << ptr_->ref_count() << " ptr:" << std::hex
                            << reinterpret_cast<size_t>(ptr_);
                delete ptr_;
                ptr_ = nullptr;
            }
            else
            {
                ptr_->decrease_ref();
                EXPECT(ptr_->ref_count() >= 1, "ref count mistake");
            }
        }
    }

    void reset(T* ptr = nullptr)
    {
        Log::info() << "reset:" << typeid(*ptr_).name()
                    << " ref count:" << ptr_->ref_count();
        if (ptr == ptr_)
            return;
        *this = SharedObject<T>(ptr);
    }

    template <typename M>
    SharedObject<M> cast()
    {
        if (auto d = dynamic_cast<M*>(ptr_))
        {
            return SharedObject<M>(d);
        }
        return SharedObject<M>(nullptr);
    }

    T& operator*()
    {
        return *ptr_;
    }
    T* operator->()
    {
        return ptr_;
    }

    T* get()
    {
        return operator->();
    }

    operator bool()
    {
        return !!(ptr_);
    }
    bool operator!()
    {
        return !bool(ptr_);
    }

    bool operator==(const SharedObject& other)
    {
        return ptr_ == other.ptr_;
    }
    bool operator!=(const SharedObject& other)
    {
        return !(*this == other);
    }

private:
    T* ptr_ = nullptr;
};

template <typename T>
inline void RemoveDiedObj(std::list<T>& ls)
{
    for (auto iter = ls.begin(); iter != ls.end();)
    {
        if (!(*iter) || !(*iter)->alive())
        {
            iter = ls.erase(iter);
            continue;
        }
        iter++;
    }
}

} // namespace games
