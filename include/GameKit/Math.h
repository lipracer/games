
namespace games
{

template <typename T>
constexpr inline T CeilDiv(T lhs, T rhs)
{
    return (lhs + rhs - 1) / rhs;
}

} // namespace games
