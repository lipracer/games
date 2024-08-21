
#include "TankObject.h"

#include <random>

#include "Animation.h"
#include "Bullet.h"
#include "GameKit/GameKit.h"
#include "TankGameMgr.h"

#define RANDOM(n) (rand() % (n))

namespace games
{
namespace tank
{
Object::Object(const Rect& rect) : rect_(rect)
{
    GAME_MGR().RegistObject(this);
}

Object::Object() : Object(Rect()) {}

Object::~Object() {}

Object* Object::SetImage(const SharedPtr<Image>& img)
{
    img_[0] = img;
    return this;
}

Object* Object::SetWarningImage(const SharedPtr<Image>& img)
{
    img_[1] = img;
    return this;
}

void Object::SetShowImage(size_t index)
{
    show_img_index_ = index;
}

Object* Object::SetRect(const Rect& rect)
{
    rect_ = rect;
    return this;
}

Object* Object::SetSpeed(int64_t speed)
{
    speed_ = speed;
    return this;
}

Object* Object::SetDirection(Direction d)
{
    EXPECT(d != Direction::kLast, "error deirection");
    direction_ = d;
    return this;
}

Object* Object::SetMap(GameMap* map)
{
    map_ = map;
    return this;
}

void Object::draw()
{
    // static double angles[] = {-PI / 2, PI / 2, 0.0, PI};
    static double angles[] = {-90.0, 90.0, 0.0, 180.0};
    img_[show_img_index_]->SetAngle(angles[static_cast<size_t>(direction_)]);
    img_[show_img_index_]->draw(rect_);
}

bool Object::TryMove()
{
    auto sv = map_->Lookup(NextLocation());
    maveable_ = true;
    sv.Foreach([&](size_t i, size_t j, GameMap::Element& e)
               { maveable_ &= (!this->BlockMe(i, j, e)); });
    return maveable_;
}

bool Object::MeetTo__Empty()
{
    return true;
}
bool Object::MeetTo__Brick()
{
    return 0;
}
bool Object::MeetTo__Block()
{
    return 0;
}
bool Object::MeetTo__Water()
{
    return 0;
}
bool Object::MeetTo__Grass()
{
    return true;
}
bool Object::MeetTo__Border()
{
    return 0;
}
bool Object::MeetTo__Tank()
{
    return true;
}
bool Object::MeetTo__Home()
{
    return 0;
}
bool Object::MeetTo__Last()
{
    return 0;
}

void Object::MoveTo_LEFT()
{
    if (direction_ != Direction::kLeft)
    {
        SetDirection(Direction::kLeft);
        return;
    }
    if (!TryMove())
        return;
    rect_.x -= speed_;
    EXPECT(rect_.x >= 0.0, "x ilegal");
    EXPECT(rect_.y >= 0.0, "y ilegal");
}

void Object::MoveTo_RIGHT()
{
    if (direction_ != Direction::kRight)
    {
        SetDirection(Direction::kRight);
        return;
    }
    if (!TryMove())
        return;
    rect_.x += speed_;
    EXPECT(rect_.x >= 0.0, "x ilegal");
    EXPECT(rect_.y >= 0.0, "y ilegal");
}

void Object::MoveTo_UP()
{
    if (direction_ != Direction::kUp)
    {
        SetDirection(Direction::kUp);
        return;
    }
    if (!TryMove())
        return;
    rect_.y -= speed_;
    EXPECT(rect_.x >= 0.0, "x ilegal");
    EXPECT(rect_.y >= 0.0, "y ilegal");
}

void Object::MoveTo_DOWN()
{
    if (direction_ != Direction::kDown)
    {
        SetDirection(Direction::kDown);
        return;
    }
    if (!TryMove())
        return;
    rect_.y += speed_;
    EXPECT(rect_.x >= 0.0, "x ilegal");
    EXPECT(rect_.y >= 0.0, "y ilegal");
}

Rect Object::NextLocation()
{
    next_location_ = rect_;
    switch (direction_)
    {
        case Direction::kLeft: next_location_.x -= speed_; break;
        case Direction::kRight: next_location_.x += speed_; break;
        case Direction::kUp: next_location_.y -= speed_; break;
        case Direction::kDown: next_location_.y += speed_; break;
        default: EXPECT(false, "error deirection");
    }
    return next_location_;
}

bool Object::BlockMe(size_t i, size_t j, GameMap::Element& e)
{
    EXPECT(i < MAP_W, "");
    EXPECT(j < MAP_H, "");
    return e != GameMap::Element::kEmpty;
}

void Object::Attack() {}

TankBase::TankBase(const Rect& rect) : Object(rect)
{
    GAME_MGR().RegistMoveableObject(this);
    prev_location_ = rect_;
    next_location_ = rect_;
}

void TankBase::Attack()
{
    auto bullet_center = rect_.center(); // BULLET_W, BULLET_H);
    switch (direction_)
    {
        case Direction::kLeft: bullet_center.x -= TANK_W / 2 + BULLET_W; break;
        case Direction::kRight: bullet_center.x += TANK_W / 2 + BULLET_W; break;
        case Direction::kUp: bullet_center.y -= TANK_H / 2 + BULLET_H; break;
        case Direction::kDown: bullet_center.y += TANK_H / 2 + BULLET_H; break;
    }
    bullet_ =
        new Bullet(dynamic_cast<EnemyTank*>(this) ? Bullet::kEvil : Bullet::kJustice,
                   Rect(bullet_center, BULLET_W, BULLET_H));
    bullet_->SetImage(GAME_MGR().GetBulletImage())
        ->SetSpeed(2)
        ->SetMap(map_)
        ->SetDirection(direction_);
    // TODO: mix all of sounds
    // GAME_MGR().PlayAttack();
}

void TankBase::die()
{
    --alive_;
    if (alive_)
    {
        DieWarning();
        return;
    }
    warning_timer_handle_.reset(nullptr);
    auto animation = CreateAnimation<ZoomAnimation>(GAME_MGR().GetBoomImage(), rect_,
                                                    std::chrono::milliseconds(1000));
    animation->Play();
    Object::die();
}

TankBase* TankBase::SetLiftCount(size_t c)
{
    alive_ = c;
    return this;
}

void TankBase::DieWarning()
{
    total_warning_frame_ = 0;
    warning_timer_handle_ = GAME_MGR().GetTimer20()->RegistListener(
        [=](size_t frame) mutable
        {
            ++total_warning_frame_;
            if (total_warning_frame_ > 10)
            {
                warning_timer_handle_ = TimerBase::TimerHandle();
                this->SetShowImage(0);
                return;
            }
            this->SetShowImage(
                /*ensure last image is norm*/ (frame + 1) % 2);
        });
    Log::info() << "regist object:" << std::hex << reinterpret_cast<size_t>(this);
}
////-----------------------------------------------////
// MyTank
////-----------------------------------------------////
MyTank::MyTank(const Rect& rect) : TankBase(rect), brithplace_(rect)
{
    message_timer_handle_ = GAME_MGR().GetTimer500()->RegistListener(this);
}

void MyTank::update(size_t tick)
{
    if (state_ == kPress)
    {
        Attack();
    }
    if (state_ == kRelease)
    {
        // SetState(kDefault);
    }
}

void MyTank::Attack()
{
    TankBase::Attack();
    SetState(kDefault);
}

void MyTank::die()
{
    TankBase::die();
    BringBackToLife();
}

void MyTank::BringBackToLife()
{
    SetRect(brithplace_);
    SetDirection(Direction::kUp);
}

void MyTank::OnAttackPress()
{
    state_ = kPress;
}

void MyTank::OnAttackRelease()
{ /*state_ = kRelease;*/
}

void MyTank::SetState(State s)
{
    state_ = s;
}

////-----------------------------------------------////
// EnemyTank
////-----------------------------------------------////
EnemyTank::EnemyTank(const Rect& rect) : TankBase(rect)
{
    direction_ = Direction::kDown;
    message_timer_handle_ = GAME_MGR().GetTimer100()->RegistListener(this);
}

void EnemyTank::update(size_t tick)
{
    if (tick % free_path_ == 0)
    {
        SetDirection(static_cast<Direction>(RANDOM(4)));
    }
    if (TryMove())
    {
        SetRect(NextLocation());
    }
    if (tick % free_path_ == 0 && (int64_t(rect_.x) % ELEMENT_W == 0)
        && (int64_t(rect_.y) % ELEMENT_W == 0))
    {
        Attack();
    }
}
} // namespace tank
} // namespace games
