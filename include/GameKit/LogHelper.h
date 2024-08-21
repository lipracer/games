#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace games
{

namespace Log
{

template <int L>
struct LogWrap
{
    template <typename T>
    LogWrap& operator<<(T&& t)
    {
        ss_ << std::forward<T>(t);
        return *this;
    }

    LogWrap(const std::string& str)
    {
        ss_ << str;
    }

    ~LogWrap()
    {
        std::cout << ss_.str() << std::endl;
    }

    std::stringstream ss_;
};

// release build type disable info log
#ifndef DEBUG
template <>
struct LogWrap<0>
{
    template <typename T>
    LogWrap& operator<<(T&& t)
    {
        return *this;
    }

    LogWrap(const std::string& str) {}

    ~LogWrap() {}
};

#endif

inline auto info()
{
    return LogWrap<0>("[INFO]");
}

inline auto warning()
{
    return LogWrap<1>("[WARN]");
}

inline auto error()
{
    return LogWrap<2>("[ERROR]");
}

} // namespace Log

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
