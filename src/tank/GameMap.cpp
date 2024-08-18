#include "GameMap.h"

#include <fstream>

#include "GameKit/GameKit.h"
#include "GameKit/LogHelper.h"
#include "TankGameMgr.h"
#include "TankObject.h"

namespace games
{
namespace tank
{
std::string GameMap::to_string(Element e)
{
    return std::to_string(static_cast<size_t>(e));
}

GameMap::GameMap()
    : map_(reinterpret_cast<Element*>(malloc(MAP_W * MAP_H * sizeof(Element))))
{
}

void GameMap::LoadMap(std::istream& ifs)
{
    std::fill_n(map_.get(), MAP_H * MAP_W, static_cast<char>(Element::kEmpty));
    for (size_t i = 0; i < MAP_W; ++i)
    {
        (*this)[0][i] = Element::kBorder;
        (*this)[MAP_H - 1][i] = Element::kBorder;
    }
    for (size_t i = 0; i < MAP_H; ++i)
    {
        (*this)[i][0] = Element::kBorder;
        (*this)[i][MAP_W - 1] = Element::kBorder;
    }
    std::string line;
    size_t row = 1;
    while (std::getline(ifs, line))
    {
        size_t first = 0, col = 1;
        do
        {
            map_[row * MAP_W + col] = atoi(line.c_str() + first);
            auto pos = line.find_first_of(' ', first);
            if (pos == std::string::npos)
            {
                break;
            }
            first = pos + 1;
            col++;
        } while (1);
        row++;
    }
}

static size_t DivAlignUp(size_t a, size_t b)
{
    return (a + b - 1) / b;
}

GameMap::SubView<GameMap::Element> GameMap::Lookup(Rect r)
{
    size_t col = r.x / ELEMENT_W + 1;
    size_t row = r.y / ELEMENT_H + 1;

    size_t col_e = (r.x + r.w - 1) / ELEMENT_W + 1;
    size_t row_e = (r.y + r.h - 1) / ELEMENT_H + 1;
    return GameMap::SubView<GameMap::Element>(map_.get() + row * MAP_W + col, row, col,
                                              row_e - row + 1, col_e - col + 1);
}

Rect GameMap::GetZoneFromCoordinate(size_t i, size_t j)
{
    return Rect((int64_t(j) - 1) * ELEMENT_W, (int64_t(i) - 1) * ELEMENT_H, ELEMENT_W,
                MAP_H);
}

Point GameMap::GetLocationFromCoordinate(size_t i, size_t j)
{
    return Point((int64_t(j) - 1) * ELEMENT_W, (int64_t(i) - 1) * ELEMENT_H);
}

std::ostream& operator<<(std::ostream& os, const GameMap::SubView<GameMap::Element>& sv)
{
    auto data = GAME_MGR().GetMap().GetData();
    size_t dis = sv.ptr_ - data;
    size_t row_i = dis / ELEMENT_W;
    size_t col_i = dis % ELEMENT_H;
    for (size_t i = 0; i < sv.row_size; ++i)
    {
        for (size_t j = 0; j < sv.col_size; ++j)
        {
            os << "[(x:" << row_i + i << ",y:" << col_i + j
               << "):" << std::to_string(static_cast<int>(sv[i][j])) << "]";
        }
        os << "\n";
    }
    return os;
}

bool AlivableObjectMap::CollsionDetection(Object* o)
{
    return CollsionDetection(o->rect());
}

bool AlivableObjectMap::CollsionDetection(const Rect& r)
{
    auto MaskValue = [](size_t s, size_t e, std::vector<StorageType>& bs)
    {
        size_t result = 0;
        size_t si = s / kBitStorageSize;
        uint64_t value = 1;
        size_t ei = (e - 1) / kBitStorageSize;
        size_t mask_count = ei > si ? (kBitStorageSize - s) : (e - s);

        value <<= mask_count;
        value -= 1;

        result = (bs[si] & (value << (s - si * kBitStorageSize)));
        if (ei > si)
        {
            value = 1;
            value <<= (e - (si + 1) * kBitStorageSize);
            value -= 1;
            result |= (bs[ei] & value);
        }
        return result;
    };
    size_t col = r.x / MIN_MAP_ELEMENT;
    size_t row = r.y / MIN_MAP_ELEMENT;

    size_t col_e = r.right() / MIN_MAP_ELEMENT;
    size_t row_e = r.bottom() / MIN_MAP_ELEMENT;
    for (; row < row_e; ++row)
    {
        if (MaskValue(col, col_e, map_[row]))
        {
            return true;
        }
    }
    return false;
}

void AlivableObjectMap::MaskMap(Object* o, const Rect& r)
{
    size_t col = r.x / MIN_MAP_ELEMENT;
    size_t row = r.y / MIN_MAP_ELEMENT;

    LocationKey lk;
    lk.location.x = col;
    lk.location.y = row;

    // o->increase_ref();
    // location_map_.emplace(lk.key, o);

    size_t col_e = r.right() / MIN_MAP_ELEMENT;
    size_t row_e = r.bottom() / MIN_MAP_ELEMENT;
    for (; row < row_e; ++row)
    {
        MaskRange_1(col, col_e, map_[row]);
    }
}

void AlivableObjectMap::ClearMap(Object* o, const Rect& r)
{
    size_t col = r.x / MIN_MAP_ELEMENT;
    size_t row = r.y / MIN_MAP_ELEMENT;

    LocationKey lk;
    lk.location.x = col;
    lk.location.y = row;

    // o->decrease_ref();
    // location_map_.erase(lk.key);

    size_t col_e = r.right() / MIN_MAP_ELEMENT;
    size_t row_e = r.bottom() / MIN_MAP_ELEMENT;

    for (; row < row_e; ++row)
    {
        MaskRange_0(col, col_e, map_[row]);
    }
}

void AlivableObjectMap::MaskRange_1(size_t s, size_t e, std::vector<StorageType>& bs)
{
    size_t si = s / kBitStorageSize;
    uint64_t value = 1;
    size_t ei = (e - 1) / kBitStorageSize;
    size_t mask_count = ei > si ? (kBitStorageSize - s) : (e - s);

    value <<= mask_count;
    value -= 1;

    bs[si] |= (value << (s - si * kBitStorageSize));
    if (ei > si)
    {
        value = 1;
        value <<= (e - (si + 1) * kBitStorageSize);
        value -= 1;
        bs[ei] |= value;
    }
}

void AlivableObjectMap::MaskRange_0(size_t s, size_t e, std::vector<StorageType>& bs)
{
    size_t si = s / kBitStorageSize;
    uint64_t value = 1;
    size_t ei = (e - 1) / kBitStorageSize;
    size_t mask_count = ei > si ? (kBitStorageSize - s) : (e - s);

    value <<= mask_count;
    value -= 1;

    bs[si] &= ~(value << (s - si * kBitStorageSize));
    if (ei > si)
    {
        value = 1;
        value <<= (e - (si + 1) * kBitStorageSize);
        value -= 1;
        bs[ei] &= ~value;
    }
}

std::tuple<size_t, size_t> AlivableObjectMap::GetLocation(const Rect& r)
{
    return std::tuple<size_t, size_t>(r.x / MIN_MAP_ELEMENT, r.y / MIN_MAP_ELEMENT);
}

std::string AlivableObjectMap::to_string()
{
    std::stringstream ss;
    for (auto& m : map_)
    {
        for (auto i : m)
        {
            std::bitset<64> bs = i;
            auto str = bs.to_string();
            ss << std::string(str.rbegin(), str.rend());
        }
        ss << "\n";
    }
    return ss.str();
}

} // namespace tank
} // namespace games
