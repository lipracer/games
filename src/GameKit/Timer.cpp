#include "GameKit/Timer.h"

namespace games
{

TimerBase::TimerBase(size_t dur) : duration_(dur), st_(std::chrono::steady_clock::now())
{
    Log::info() << "construct timer duration:" << duration_;
}

TimerBase::~TimerBase()
{
    clear();
}

void TimerBase::reset()
{
    tick_ = 0;
    st_ = std::chrono::steady_clock::now();
}

TimerBase::TimerHandle TimerBase::RegistListener(const TimerBase::ListenerFunc& func)
{
    listener_.emplace_back(func, true);
    auto iter = listener_.end();
    return std::make_unique<TimerBase::__TimerHandle>(this, --iter);
}

void TimerBase::UnregistListener(TimerId id)
{
    id->active = false;
}

TimerBase::TimerHandle TimerBase::RegistListener(ObjectBase* obj)
{
    Log::info() << "regist object:" << std::hex << reinterpret_cast<size_t>(obj);
    return RegistListener([=](size_t tick) mutable { obj->update(tick); });
}

bool TimerBase::update()
{
    auto cur_st_ = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::milliseconds>(cur_st_ - st_).count()
        > duration_)
    {
        st_ = cur_st_;
        GC_Object();
        for (auto& it : listener_)
        {
            if (it.active)
            {
                it.handle(tick_++);
            }
        }
        return true;
    }
    return false;
}

void TimerBase::GC_Object()
{
    for (auto iter = listener_.begin(); iter != listener_.end();)
    {
        if (!iter->active)
        {
            iter = listener_.erase(iter);
            continue;
        }
        iter++;
    }
}

void TimerBase::clear()
{
    Log::info() << "clear timer(" << duration_ << ") listener size:" << listener_.size();
    listener_.clear();
}

} // namespace games
