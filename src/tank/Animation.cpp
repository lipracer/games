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

AnimationBase::AnimationBase(size_t frames, const std::function<void(size_t)>& func,
                             const std::function<void(void)>& end_func)
    : ObjectBase(), frames_(frames), func_(func), end_func_(end_func)
{
    update_handle_ = GAME_MGR().GetTimer10()->RegistListener(this);
}

void AnimationBase::Play()
{
    PlayAfter(std::chrono::milliseconds(0));
}

void AnimationBase::update(size_t tick)
{
    if (this->animation_frame_ < this->frames_)
    {
        auto cur_dur = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - this->start_pt_);
        if (cur_dur > delay_)
        {
            func_(animation_frame_);
            animation_frame_++;
        }
        return;
    }
    if (end_func_)
        end_func_();
    this->EndPlay();
}

void AnimationBase::PlayAfter(std::chrono::milliseconds dur)
{
    delay_ = dur;
}

void AnimationBase::EndPlay()
{
    update_handle_.reset(nullptr);
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
    : Animation<ZoomAnimation>(20),
      img_(img),
      rect_(rect),
      cur_rect_(rect.center(), 0.0, 0.0)
{
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
