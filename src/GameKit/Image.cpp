#include "GameKit/Image.h"

#include <fstream>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_main.h>

#include "GameKit/LogHelper.h"

namespace games
{
template <>
SDL_FRect CastToSdl<SDL_FRect>(const Rect& rect)
{
    SDL_FRect result;
    result.x = rect.x;
    result.y = rect.y;
    result.w = rect.w;
    result.h = rect.h;
    return result;
}

Image::Image(void* renderer, const std::string& name) : renderer(renderer)
{
    SDL_Surface* loadedSurface = SDL_LoadBMP(name.c_str());
    if (!loadedSurface)
        return;

    texture = SDL_CreateTextureFromSurface(reinterpret_cast<SDL_Renderer*>(renderer),
                                           loadedSurface);
    SDL_DestroySurface(loadedSurface); // Free the surface after creating texture
}

void Image::draw(const Rect& rect)
{
    if (!texture)
        std::cerr << __func__ << " empty texture" << std::endl;
    auto sdl_rect = CastToSdl<SDL_FRect>(rect);
    if (SDL_fabsf(rect.w) < 0.000001)
    {
        return;
    }
    SDL_RenderTextureRotated(reinterpret_cast<SDL_Renderer*>(renderer),
                             reinterpret_cast<SDL_Texture*>(texture), nullptr,
                             rect.valid() ? &sdl_rect : nullptr, angle_, nullptr,
                             SDL_FLIP_NONE);
    // SDL_RenderTexture(reinterpret_cast<SDL_Renderer*>(renderer),
    //                   reinterpret_cast<SDL_Texture*>(texture), nullptr,
    //                   rect.valid() ? &sdl_rect : nullptr);
}

Sound::Sound(const std::string& file)
{
    std::ifstream ifs(file);
    EXPECT(ifs.is_open(), "can't not open file!");

    ifs.seekg(0, std::ios::end);
    int64_t file_length = ifs.tellg();
    // file_length = 44100;

    const SDL_AudioSpec spec = {SDL_AUDIO_S16, 2, 44100};
    stream_ =
        SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    EXPECT(stream_, "can't not open stream!");

    ifs.seekg(0, std::ios::beg);
    buffer_.resize(file_length);
    ifs.read(buffer_.data(), buffer_.size());

    ifs.close();
}

Sound::~Sound()
{
    // SDL_DestroyAudioStream(reinterpret_cast<SDL_AudioStream*>(stream_));
}

void Sound::play()
{
    SDL_PutAudioStreamData(reinterpret_cast<SDL_AudioStream*>(stream_), buffer_.data(),
                           buffer_.size());

    Sint16 onesample = 22;
    SDL_PutAudioStreamData(reinterpret_cast<SDL_AudioStream*>(stream_), &onesample,
                           sizeof(Sint16));

    SDL_ResumeAudioDevice(
        SDL_GetAudioStreamDevice(reinterpret_cast<SDL_AudioStream*>(stream_)));
}
} // namespace games
