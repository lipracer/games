#include "Animation.h"

#include "TankGameMgr.h"

namespace games
{
namespace tank
{
size_t Animation::MillPerFrame()
{
    return 20;
}

Animation::Animation(size_t frames) : ObjectBase(), frames_(frames) {}

void Animation::Play()
{
    PlayAfter(std::chrono::milliseconds(0));
}

void Animation::update()
{
    if (this->animation_frame_++ < this->frames_)
    {
        auto cur_dur = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - this->start_pt_);
        if (cur_dur > delay_)
        {
            animation_frame_++;
        }
        return;
    }
    this->EndPlay();
}

void Animation::PlayAfter(std::chrono::milliseconds dur)
{
    delay_ = dur;
}

void Animation::EndPlay()
{
    die();
}

BlinkAnimation::BlinkAnimation(const SharedPtr<Image>& img, const Rect& rect,
                               std::chrono::milliseconds dur)
    : Animation(50), img_(img)
{
}

void BlinkAnimation::draw()
{
    img_->draw();
}

void BlinkAnimation::update() {}

ZoomAnimation::ZoomAnimation(const SharedPtr<Image>& img, const Rect& rect,
                             std::chrono::milliseconds dur)
    : Animation(50), img_(img), rect_(rect), cur_rect_(rect.center(), 0.0, 0.0)
{
    GAME_MGR().GetTimer10()->RegistListener(this);
}

void ZoomAnimation::draw()
{
    img_->draw(cur_rect_);
}

void ZoomAnimation::update()
{
    Animation::update();
    float z = CurFrame();
    z = z / Frames();
    cur_rect_ = Rect(cur_rect_.center(), rect_.w * z, rect_.h * z);
}
} // namespace tank
} // namespace games
