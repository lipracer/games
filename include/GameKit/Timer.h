#pragma once

#include <chrono>
#include <list>

#include "GameKit/Object.h"

namespace games
{

template <typename DeriveT>
class TimerBase
{
public:
    TimerBase() : st_(std::chrono::steady_clock::now()) {}

    void reset()
    {
        st_ = std::chrono::steady_clock::now();
    }

    void RegistListener(ObjectBase* listener)
    {
        listener_.push_back(listener);
    }

    bool update()
    {
        auto cur_st_ = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(cur_st_ - st_).count()
            > DeriveT::Duration)
        {
            st_ = cur_st_;
            for (auto iter = listener_.begin(); iter != listener_.end(); iter++)
            {
                if (*iter && (*iter)->alive())
                {
                    (*iter)->update();
                }
            }
            RemoveDiedObj(listener_);
            return true;
        }
        return false;
    }

    void clear()
    {
        RemoveDiedObj(listener_);
    }

private:
    std::list<SharedObject<ObjectBase>> listener_;
    decltype(std::chrono::steady_clock::now()) st_;
};

class Timer10 : public TimerBase<Timer10>
{
public:
    constexpr static size_t Duration = 10;
};

class Timer20 : public TimerBase<Timer20>
{
public:
    constexpr static size_t Duration = 20;
};

class Timer100 : public TimerBase<Timer100>
{
public:
    constexpr static size_t Duration = 100;
};

class Timer500 : public TimerBase<Timer500>
{
public:
    constexpr static size_t Duration = 500;
};

class Timer1000 : public TimerBase<Timer1000>
{
public:
    constexpr static size_t Duration = 1000;
};

class Timer5000 : public TimerBase<Timer5000>
{
public:
    constexpr static size_t Duration = 5000;
};

} // namespace games
