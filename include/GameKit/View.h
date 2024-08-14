#pragma once

#include <iosfwd>

namespace games
{

struct Point
{
    float x;
    float y;
    Point() {};
    Point(float x, float y) : x(x), y(y) {}
};

struct Rect
{
    float x;
    float y;
    float w;
    float h;

    Rect() : Rect(-1, -1, -1, -1) {}

    Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

    Rect(const Point& center, float w, float h)
        : x(center.x - w / 2), y(center.y - h / 2), w(w), h(h)
    {
    }

    bool valid() const
    {
        return w > 0 && h > 0;
    }

    Point center() const
    {
        return Point(x + w / 2, y + h / 2);
    }

    float right() const
    {
        return x + w;
    }

    float bottom() const
    {
        return y + h;
    }

    bool overlap(const Rect& other) const
    {
        return !(x >= other.right() | (y >= other.bottom()) | (right() <= other.x)
                 | (bottom() <= other.y));
    }
};

std::ostream& operator<<(std::ostream& os, const Point&);
std::ostream& operator<<(std::ostream& os, const Rect&);

} // namespace games
