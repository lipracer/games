#include "TankObject.h"
namespace games
{
namespace tank
{
class Bullet : public Object
{
public:
    enum Kind
    {
        // my tank's bullet
        kJustice,
        // enemy tank's bullet
        kEvil,
    };
    Bullet(Kind, const Rect& rect);

    bool BlockMe(size_t, size_t, GameMap::Element&) override;

    void die() override;

    void update(size_t tick) override;

    bool MeetToDie(Object*);

    void DisableAnimation()
    {
        die_animation_ = false;
    }

    void PlayBoom(const Rect& rect);

    std::string name() override
    {
        return "Bullet";
    }

private:
    std::vector<std::tuple<size_t, size_t>> blockme_locations_;
    Kind kind_;
    bool die_animation_ = true;
    TimerBase::TimerHandle update_handle_;
};
} // namespace tank
} // namespace games
