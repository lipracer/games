#include <cmath>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "GameKit/GameMgr.h"
#include "tank/TankGameMgr.h"

REGIST_GAME_MANAGER("tank", []() { return new games::tank::TankGameMgr(); })

struct AppContext
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_bool app_quit = SDL_FALSE;
};

int SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return -1;
}

inline games::GameMgr* CurrentMgr()
{
    return games::GameContext::instance().CurrentGameMgr();
}

int SDL_AppInit(void** appstate, int argc, char* argv[])
{
    auto game_list = games::GameContext::instance().GameList();
    size_t game_index = 1;
    std::cout << "game menu:" << std::endl;
    for (auto game : game_list)
    {
        std::cout << game_index << " " << game << std::endl;
    }
    std::cout << "please input game index:" << std::endl;
    std::cin >> game_index;
    if (game_index - 1 >= game_list.size())
    {
        std::cerr << "invalid index:" << game_index << std::endl;
        exit(1);
    }
    games::GameContext::instance().SwitchGame(game_list[game_index - 1]);
    // init the library, here we make a window so we only need the Video
    // capabilities.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        return SDL_Fail();
    }

    // create a window
    SDL_Window* window = SDL_CreateWindow("Window", CurrentMgr()->Window_W(),
                                          CurrentMgr()->Window_H(), SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        return SDL_Fail();
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        return SDL_Fail();
    }

    CurrentMgr()->InitGame(renderer);

    // print some information about the window
    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth)
        {
            SDL_Log("This is a highdpi environment.");
        }
    }

    // set up the application data
    *appstate = new AppContext{
        window,
        renderer,
    };

    SDL_Log("Application started successfully!");

    return 0;
}

int SDL_AppEvent(void* appstate, const SDL_Event* event)
{
    auto* app = (AppContext*)appstate;

#define ON_KEYWARD_EVENT(e, d) \
    case SDLK_##d: CurrentMgr()->Dispatch_##d##_##e(); break;

    if (event->type == SDL_EVENT_QUIT)
    {
        app->app_quit = SDL_TRUE;
        return 0;
    }
    if (CurrentMgr() && CurrentMgr()->available())
    {
        if (event->type == SDL_EVENT_KEY_DOWN)
        {
            switch (event->key.key)
            {
                ON_KEYWARD_EVENT(Press, LEFT)
                ON_KEYWARD_EVENT(Press, RIGHT)
                ON_KEYWARD_EVENT(Press, UP)
                ON_KEYWARD_EVENT(Press, DOWN)
                ON_KEYWARD_EVENT(Press, Q)
                ON_KEYWARD_EVENT(Press, A)
                ON_KEYWARD_EVENT(Press, 0)
                ON_KEYWARD_EVENT(Press, 1)
                ON_KEYWARD_EVENT(Press, 2)
                ON_KEYWARD_EVENT(Press, 3)
                ON_KEYWARD_EVENT(Press, 4)
            }
        }
        else if (event->type == SDL_EVENT_KEY_UP)
        {
            switch (event->key.key)
            {
                ON_KEYWARD_EVENT(Release, LEFT)
                ON_KEYWARD_EVENT(Release, RIGHT)
                ON_KEYWARD_EVENT(Release, UP)
                ON_KEYWARD_EVENT(Release, DOWN)
                ON_KEYWARD_EVENT(Release, A)
            }
        }
    }

    return 0;
}

int SDL_AppIterate(void* appstate)
{
    auto* app = (AppContext*)appstate;

    // draw a color
    // auto time = SDL_GetTicks() / 1000.f;
    // auto red = (std::sin(time) + 1) / 2.0 * 255;
    // auto green = (std::sin(time / 2) + 1) / 2.0 * 255;
    // auto blue = (std::sin(time) * 2 + 1) / 2.0 * 255;

    // SDL_SetRenderDrawColor(app->renderer, red, green, blue,
    // SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    CurrentMgr()->DispatchMessage();

    // auto x = SDL_GetTicks() / 100 % tank::WINDOW_W;
    // tank::gMyTank->draw(tank::Rect(x, 0, tank::TANK_W, tank::TANK_H));

    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

void SDL_AppQuit(void* appstate)
{
    auto* app = (AppContext*)appstate;
    if (app)
    {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
