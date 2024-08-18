#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace games
{

namespace Log
{
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
inline LogWrap info()
{
    return LogWrap("[INFO]");
}

inline LogWrap warning()
{
    return LogWrap("[WARN]");
}

inline LogWrap error()
{
    return LogWrap("[ERROR]");
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
