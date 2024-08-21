#pragma once

#include <chrono>
#include <functional>
#include <list>
#include <memory>

#include "GameKit/Object.h"

namespace games
{

class TimerBase : public std::enable_shared_from_this<TimerBase>
{
    using ListenerFunc = std::function<void(size_t)>;
    struct Listener
    {
        Listener(const ListenerFunc& handle, bool active) : handle(handle), active(active)
        {
        }
        ListenerFunc handle;
        bool active;
    };

public:
    using TimerId = typename std::list<Listener>::iterator;
    struct __TimerHandle
    {
        std::list<Listener>::iterator iter;
        TimerBase* timer;
        __TimerHandle(TimerBase* timer, std::list<Listener>::iterator iter)
            : timer(timer), iter(iter)
        {
        }
        __TimerHandle() : __TimerHandle({}, {}) {}
        __TimerHandle(const __TimerHandle&) = delete;

        void release()
        {
            if (timer)
            {
                timer->UnregistListener(iter);
            }
        }
        ~__TimerHandle()
        {
            release();
        }
    };

    using TimerHandle = std::unique_ptr<__TimerHandle>;

    TimerBase(size_t dur);

    ~TimerBase();

    void reset();

    TimerHandle RegistListener(const ListenerFunc& func);
    TimerHandle RegistListener(ObjectBase* obj);

    void UnregistListener(TimerId id);

    void GC_Object();

    bool update();

    void clear();

    auto CurrentTP()
    {
        return st_;
    }

private:
    std::list<Listener> listener_;
    size_t duration_;
    decltype(std::chrono::steady_clock::now()) st_;
    size_t tick_;
};

class Timer10 : public TimerBase
{
public:
    Timer10() : TimerBase(10) {}
};

class Timer20 : public TimerBase
{
public:
    Timer20() : TimerBase(20) {}
};

class Timer100 : public TimerBase
{
public:
    Timer100() : TimerBase(100) {}
};

class Timer500 : public TimerBase
{
public:
    Timer500() : TimerBase(500) {}
};

class Timer1000 : public TimerBase
{
public:
    Timer1000() : TimerBase(1000) {}
};

class Timer5000 : public TimerBase
{
public:
    Timer5000() : TimerBase(5000) {}
};

} // namespace games
