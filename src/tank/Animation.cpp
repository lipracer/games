#include "Animation.h"

#include "TankGameMgr.h"

namespace games
{
namespace tank
{
size_t AnimationBase::MillPerFrame()
{
    return 20;
}

AnimationBase::AnimationBase(size_t frames, const std::function<void(void)>& func)
    : ObjectBase(), frames_(frames), func_(func)
{
}

void AnimationBase::Play()
{
    PlayAfter(std::chrono::milliseconds(0));
}

void AnimationBase::update()
{
    if (this->animation_frame_++ < this->frames_)
    {
        auto cur_dur = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - this->start_pt_);
        if (cur_dur > delay_)
        {
            animation_frame_++;
            func_();
        }
        return;
    }
    this->EndPlay();
}

void AnimationBase::PlayAfter(std::chrono::milliseconds dur)
{
    delay_ = dur;
}

void AnimationBase::EndPlay()
{
    die();
}

BlinkAnimation::BlinkAnimation(const SharedPtr<Image>& img, const Rect& rect,
                               std::chrono::milliseconds dur)
    : Animation<BlinkAnimation>(50), img_(img)
{
}

void BlinkAnimation::draw()
{
    img_->draw();
}

void BlinkAnimation::change() {}

ZoomAnimation::ZoomAnimation(const SharedPtr<Image>& img, const Rect& rect,
                             std::chrono::milliseconds dur)
    : Animation<ZoomAnimation>(50), img_(img), rect_(rect), cur_rect_(rect.center(), 0.0, 0.0)
{
    GAME_MGR().GetTimer10()->RegistListener(this);
}

void ZoomAnimation::draw()
{
    img_->draw(cur_rect_);
}

void ZoomAnimation::change()
{
    float z = CurFrame();
    z = z / Frames();
    cur_rect_ = Rect(cur_rect_.center(), rect_.w * z, rect_.h * z);
}
} // namespace tank
} // namespace games
