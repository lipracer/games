#pragma once

#include <memory>

#include "GameKit/GameKit.h"
#include "GameKit/Image.h"
#include "GameKit/Object.h"
#include "GameKit/View.h"
#include "GameMap.h"

namespace games
{
namespace tank
{
template <typename T>
using SharedPtr = std::shared_ptr<T>;

enum class Direction
{
    kLeft,
    kRight,
    kUp,
    kDown,
    kLast
};

class Object : public ObjectBase
{
public:
    Object();
    Object(const Rect& rect);
    virtual ~Object();

    Object* SetImage(const SharedPtr<Image>& img);
    Object* SetRect(const Rect& rect);
    Object* SetSpeed(int64_t speed);
    Object* SetDirection(Direction d);
    Object* RegistOnKeywardEvent(size_t index);
    Object* SetMap(GameMap* map);

    const Rect& rect()
    {
        return rect_;
    }
    const Rect& PrevRect()
    {
        return prev_location_;
    }
    void UpdatePrevRect()
    {
        prev_location_ = rect_;
    }

    void draw() override;
    void update() override {}
    std::string name() override
    {
        return "Object";
    }

    virtual void MoveTo_LEFT();
    virtual void MoveTo_RIGHT();
    virtual void MoveTo_UP();
    virtual void MoveTo_DOWN();
    virtual bool BlockMe(size_t i, size_t j, GameMap::Element&);
    virtual void Attack();

#define DEF_MEETTO_METHOD(_) virtual bool MeetTo__##_()
    DEF_ELEMENT_LIST_0(DEF_MEETTO_METHOD)

    bool TryMove();

    bool Moveable()
    {
        return maveable_;
    }
    void SetMoveable(bool m)
    {
        maveable_ = m;
    }

    Point CenterPoint()
    {
        Point p;
        p.x = rect_.x + rect_.w / 2;
        p.y = rect_.y + rect_.h / 2;
        return p;
    }

    Rect NextLocation();
    Rect NextRect()
    {
        return next_location_;
    }

protected:
    Rect rect_;
    Rect next_location_;
    Rect prev_location_;
    SharedPtr<Image> img_;
    int64_t speed_ = 0;
    Direction direction_ = Direction::kUp;
    GameMap* map_ = nullptr;
    bool maveable_ = true;
};

class HomeObject : public Object
{
public:
    void die() override {}
};

class Bullet;

class TankBase : public Object
{
public:
    TankBase(const Rect& rect);
    void update() override;
    void Attack() override;
    void die() override;
    TankBase* SetLiftCount(size_t c)
    {
        alive_ = c;
        return this;
    }

    std::string name() override
    {
        return "TankBase";
    }

protected:
    Bullet* bullet_ = nullptr;
    size_t update_tick_ = 0;
};

class MyTank : public TankBase
{
    enum State
    {
        kDefault,
        kPress,
        kRelease,
    };

public:
    MyTank(const Rect& rect) : TankBase(rect), brithplace_(rect) {}

    void update() override;
    void Attack() override;
    void die() override;

    void OnAttackPress();
    void OnAttackRelease();
    void SetState(State s);

    void BringBackToLife();

private:
    Rect brithplace_;
    State state_ = kDefault;
};

class EnemyTank : public TankBase
{
public:
    EnemyTank(const Rect& rect) : TankBase(rect)
    {
        direction_ = Direction::kDown;
    }
    void update() override;
    void enable_cd()
    {
        enable_collsion_detection_ = true;
    }
    bool collsion_detection()
    {
        return enable_collsion_detection_;
    }

    EnemyTank* SetFreePath(size_t fp)
    {
        free_path_ = fp;
        return this;
    }

    std::string name() override
    {
        return "EnemyTank";
    }

private:
    bool enable_collsion_detection_ = false;
    size_t free_path_ = 8;
};

} // namespace tank
} // namespace games
