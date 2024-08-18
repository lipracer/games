#pragma once

#include <bitset>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "GameKit/GameKit.h"

#define DEF_ELEMENT_LIST(_) \
    _(Empty), _(Brick), _(Block), _(Water), _(Grass), _(Border), _(Tank), _(Home), _(Last)

#define DEF_ELEMENT_LIST_0(_) \
    _(Empty);                 \
    _(Brick);                 \
    _(Block);                 \
    _(Water);                 \
    _(Grass);                 \
    _(Border);                \
    _(Tank);                  \
    _(Home);                  \
    _(Last);

#define DEF_ELEMENT_ITER(_) k##_

namespace games
{
namespace tank
{
enum
{
    // game window width
    WINDOW_W = 520,
    // game window hight
    WINDOW_H = 520,
    TANK_W = 40,
    TANK_H = 40,
    // element count
    MAP_W = 13 * 2 + 2,
    MAP_H = MAP_W,
    // element width hald of tank width
    ELEMENT_W = 20,
    ELEMENT_H = 20,
    BULLET_W = 5,
    BULLET_H = 5,
    MIN_MAP_ELEMENT = 5,
};

static_assert(WINDOW_W % MIN_MAP_ELEMENT == 0 && WINDOW_H % MIN_MAP_ELEMENT == 0, "");

class GameMap
{
public:
    struct Element
    {
        constexpr static size_t SubElementLength = 8;
        using StorageType = unsigned char;
        enum EM
        {
            DEF_ELEMENT_LIST(DEF_ELEMENT_ITER)
        };
        Element(EM m) : data_(static_cast<StorageType>(m)) {}

        template <typename T>
        Element(T t) : data_(t)
        {
        }

        template <typename T>
        Element& operator=(T t)
        {
            data_ = t;
            return *this;
        }
        bool operator==(EM m)
        {
            return static_cast<StorageType>(m) == data_;
        }
        bool operator!=(EM m)
        {
            return static_cast<StorageType>(m) != data_;
        }
        operator EM()
        {
            return static_cast<EM>(data_);
        }
        friend inline std::ostream& operator<<(std::ostream& os, Element e)
        {
            os << std::to_string(static_cast<int>(e.data_));
            return os;
        }
        StorageType data_ = kEmpty;
    };
    static std::string to_string(Element);
    struct header
    {
        char name[7];
        int pass;
    };

    template <typename ET>
    struct Adapter
    {
        Adapter(ET* ptr) : ptr_(ptr) {}
        Element& operator[](size_t index) const
        {
            return reinterpret_cast<ET&>(ptr_[index]);
        }

    private:
        ET* ptr_;
    };

    template <typename ET>
    struct SubView
    {
        SubView(ET* ptr, size_t ri, size_t ci, size_t rs, size_t cs)
            : ptr_(ptr), row_index(ri), col_index(ci), row_size(rs), col_size(cs)
        {
        }
        Adapter<Element> operator[](size_t index) const
        {
            return ptr_ + index * MAP_W;
        }

        void Foreach(const std::function<void(size_t, size_t, Element&)>& func)
        {
            for (size_t i = 0; i < row_size; ++i)
            {
                for (size_t j = 0; j < col_size; ++j)
                {
                    func(row_index + i, col_index + j, (*this)[i][j]);
                }
            }
        }

        ET* ptr_;
        size_t row_index;
        size_t col_index;
        size_t row_size;
        size_t col_size;
        friend std::ostream& operator<<(std::ostream& os, const SubView<Element>& sv);
    };

    GameMap();

    void LoadMap(std::istream& ifs);

    void WriteToFile(const std::string& name);

    Adapter<Element> operator[](size_t index)
    {
        return map_.get() + index * MAP_W;
    }

    SubView<Element> Lookup(Rect r);

    Rect GetZoneFromCoordinate(size_t i, size_t j);

    Point GetLocationFromCoordinate(size_t i, size_t j);

    Element* GetData()
    {
        return map_.get();
    }

private:
    int pass = 0;
    std::unique_ptr<Element[]> map_;
};

class Object;

class AlivableObjectMap
{
public:
    union LocationKey
    {
        size_t key;
        struct
        {
            size_t x : 32;
            size_t y : 32;
        } location;
    };

    using StorageType = uint64_t;
    constexpr static size_t kMapLength = WINDOW_W / MIN_MAP_ELEMENT;
    constexpr static size_t kBitStorageSize = sizeof(StorageType) * 8;
    constexpr static size_t kVectorLength = CeilDiv(kMapLength, kBitStorageSize);
    AlivableObjectMap() : map_(kMapLength, std::vector<StorageType>(kVectorLength, 0)) {}

    bool CollsionDetection(Object* o);

    bool CollsionDetection(const Rect& r);

    void MaskMap(Object* o, const Rect& r);

    void ClearMap(Object* o, const Rect& r);

    void MaskRange_1(size_t s, size_t e, std::vector<StorageType>& bs);

    void MaskRange_0(size_t s, size_t e, std::vector<StorageType>& bs);

    std::tuple<size_t, size_t> GetLocation(const Rect& r);

    std::string to_string();

private:
    std::vector<std::vector<StorageType>> map_;
    // std::unordered_map<size_t, Object*> location_map_;
};

} // namespace tank
} // namespace games
