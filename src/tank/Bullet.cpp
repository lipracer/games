#include "Bullet.h"

#include "Animation.h"
#include "GameKit/GameKit.h"
#include "TankGameMgr.h"

namespace games
{
namespace tank
{
Bullet::Bullet(Kind kind, const Rect& rect) : Object(rect), kind_(kind)
{
    GAME_MGR().RegistMoveableObject(this);
    update_handle_ = GAME_MGR().GetTimer10()->RegistListener(this);
    blockme_locations_.reserve(4);
    prev_location_ = rect_;
}

void Bullet::update(size_t tick)
{
    EXPECT(alive(), std::string("expect object alive:"));
    if (!TryMove())
    {
        this->die();
        return;
    }
    Object::SetRect(NextLocation());
}

bool Bullet::MeetToDie(Object* obj)
{
    if (auto b = dynamic_cast<Bullet*>(obj))
    {
        b->DisableAnimation();
        b->die();
        DisableAnimation();
        auto lhs = rect().center();
        auto rhs = b->rect().center();
        PlayBoom(
            Rect(Point((lhs.x + rhs.x) / 2, (lhs.y + rhs.y) / 2), ELEMENT_W, ELEMENT_H));
        die();
        return true;
    }
    if (kind_ == kEvil && !dynamic_cast<EnemyTank*>(obj))
    {
        DisableAnimation();
        obj->die();
        die();
        return true;
    }
    else if (kind_ == kJustice && dynamic_cast<EnemyTank*>(obj))
    {
        DisableAnimation();
        obj->die();
        die();
        return true;
    }
    else
    {
        die();
    }
    return false;
}

void Bullet::die()
{
    Log::info() << "die bullet:" << std::hex << reinterpret_cast<size_t>(this);
    update_handle_.reset(nullptr);
    Object::die();
    if (!die_animation_)
    {
        return;
    }
    auto center = rect_.center();
    auto boom_rect = Rect(center, 20, 20);
    if (!blockme_locations_.empty())
    {
        auto min_x = std::min_element(blockme_locations_.begin(),
                                      blockme_locations_.end(), [](auto& lhs, auto& rhs)
                                      { return std::get<0>(lhs) < std::get<0>(rhs); });

        auto max_x = std::max_element(blockme_locations_.begin(),
                                      blockme_locations_.end(), [](auto& lhs, auto& rhs)
                                      { return std::get<0>(lhs) < std::get<0>(rhs); });

        auto min_y = std::min_element(blockme_locations_.begin(),
                                      blockme_locations_.end(), [](auto& lhs, auto& rhs)
                                      { return std::get<1>(lhs) < std::get<1>(rhs); });

        auto max_y = std::max_element(blockme_locations_.begin(),
                                      blockme_locations_.end(), [](auto& lhs, auto& rhs)
                                      { return std::get<1>(lhs), std::get<1>(rhs); });
        auto min_r =
            map_->GetZoneFromCoordinate(std::get<0>(*min_x), std::get<1>(*min_y));
        auto max_r =
            map_->GetZoneFromCoordinate(std::get<0>(*max_x), std::get<1>(*max_y));
        auto min_zone = min_r.center();
        auto max_zone = max_r.center();
        center = Point((min_zone.x + max_zone.x) / 2, (min_zone.y + max_zone.y) / 2);
        boom_rect =
            Rect(center, max_r.x - min_r.x + ELEMENT_W, max_r.y - min_r.y + ELEMENT_H);
    }
    PlayBoom(boom_rect);
}

bool Bullet::BlockMe(size_t i, size_t j, GameMap::Element& e)
{
    switch (e)
    {
        case GameMap::Element::kBrick:
        {
            blockme_locations_.emplace_back(i, j);
            e = GameMap::Element::kEmpty;
            return true;
        }
        case GameMap::Element::kBlock: return true;
        case GameMap::Element::kBorder:
        {
            return true;
        }
        case GameMap::Element::kHome:
        {
            GAME_MGR().DestroyHome();
            GAME_MGR().EmitMessage(GameMessage::kGameOver);
            return true;
        }
    }
    return false;
}

void Bullet::PlayBoom(const Rect& rect)
{
    auto animation = CreateAnimation<ZoomAnimation>(GAME_MGR().GetBoomImage(), rect,
                                                    std::chrono::milliseconds(1000));
    animation->Play();
}
} // namespace tank
} // namespace games
