#pragma once

#include <iostream>
#include <sstream>

namespace games
{

struct LogWrap
{
    template <typename T>
    LogWrap& operator<<(T&& t)
    {
        ss_ << std::forward<T>(t);
        return *this;
    }

    ~LogWrap()
    {
        std::cout << ss_.str() << std::endl;
    }

    std::istringstream ss_;
};

#define EXPECT(p, msg)                            \
    do                                            \
    {                                             \
        if (!(p))                                 \
        {                                         \
            auto m = std::string(__FILE__);       \
            m += ":";                             \
            m += std::to_string(__LINE__) + "\n"; \
            m += msg;                             \
            std::cout << m << std::endl;          \
            int n = 0;                            \
            *reinterpret_cast<int*>(n) = 0;       \
        }                                         \
    } while (0);

} // namespace games
