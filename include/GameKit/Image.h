#pragma once

#include <vector>

#include "GameKit/View.h"

namespace games
{
class Image
{
public:
    Image(void* renderer, const std::string& name);

    /// draw image on screen
    void draw(const Rect& rect = {});

    void SetAngle(double angle)
    {
        angle_ = angle;
    }

    void SetBackground(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

private:
    void* renderer_ = nullptr;
    void* texture_ = nullptr;
    double angle_ = 0.0;
};

class Sound
{
public:
    Sound(const std::string& file);
    ~Sound();

    void play();

private:
    std::vector<char> buffer_;
    void* stream_ = nullptr;
};

template <typename SDLRectT>
SDLRectT CastToSdl(const Rect& rect);

} // namespace games
