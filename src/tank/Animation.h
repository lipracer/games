#include "TankGameMgr.h"
#include "TankObject.h"

namespace games
{
namespace tank
{
class AnimationBase : public ObjectBase
{
public:
    static size_t MillPerFrame();

    AnimationBase(size_t frames, const std::function<void(void)>& func);

    void Play();
    void PlayAfter(std::chrono::milliseconds dur);

    size_t CurFrame() const
    {
        return animation_frame_;
    }

    size_t Frames() const
    {
        return frames_;
    }

    void EndPlay();

    void update() final;

    void draw() override {}

    std::string name() override
    {
        return "AnimationBase";
    }

protected:
    size_t frames_ = 0;
    size_t animation_frame_ = 0;
    decltype(std::chrono::steady_clock::now()) start_pt_;
    std::chrono::milliseconds delay_;
    std::function<void(void)> func_;
};

template <typename T>
class Animation : public AnimationBase
{
public:
    Animation(size_t frames)
        : AnimationBase(frames, [=]() { static_cast<T*>(this)->change(); })
    {
    }
};

class BlinkAnimation : public Animation<BlinkAnimation>
{
public:
    BlinkAnimation(const SharedPtr<Image>& img, const Rect& rect,
                   std::chrono::milliseconds dur);

    void draw() override;
    void change();

private:
    SharedPtr<Image> img_;
};

class ZoomAnimation : public Animation<ZoomAnimation>
{
public:
    ZoomAnimation(const SharedPtr<Image>& img, const Rect& rect,
                  std::chrono::milliseconds dur);
    void draw() override;
    void change();

private:
    SharedPtr<Image> img_;
    Rect rect_;
    Rect cur_rect_;
};

template <typename T, typename... Args>
static auto CreateAnimation(Args... args)
{
    auto ptr = new T(std::forward<Args>(args)...);
    GAME_MGR().RegistObject(ptr);
    return ptr;
}
} // namespace tank
} // namespace games
