#include "GameKit/View.h"

#include <sstream>

namespace games
{

std::ostream& operator<<(std::ostream& os, const Point& p)
{
    os << "Point(" << p.x << "," << p.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Rect& r)
{
    os << "Point(" << r.x << "," << r.y << "," << r.right() << "," << r.bottom() << ")";
    return os;
}

} // namespace games
