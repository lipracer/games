#include "GameKit/Object.h"

namespace games
{
size_t ObjectBase::gMemoryStatistic = 0;

ObjectBase::ObjectBase()
{
    gMemoryStatistic++;
}

ObjectBase::~ObjectBase()
{
    gMemoryStatistic--;
}

} // namespace games
