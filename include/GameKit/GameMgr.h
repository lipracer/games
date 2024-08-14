#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "GameKit/LogHelper.h"

namespace games
{

#define GAME_KEY_LIST_0(_)                                                             \
    _(LEFT), _(RIGHT), _(UP), _(DOWN), _(A), _(Q), _(0), _(1), _(2), _(3), _(4), _(5), \
        _(6), _(7), _(8), _(9)

#define GAME_KEY_LIST_1(_) \
    _(LEFT);               \
    _(RIGHT);              \
    _(UP);                 \
    _(DOWN);               \
    _(A);                  \
    _(Q);                  \
    _(0);                  \
    _(1);                  \
    _(2);                  \
    _(3);                  \
    _(4);                  \
    _(5);                  \
    _(6);                  \
    _(7);                  \
    _(8);                  \
    _(9)

class GameMgr
{
public:
    static GameMgr* current();
    virtual void DispatchMessage() = 0;
    virtual ~GameMgr() {}

    virtual size_t Window_X() = 0;
    virtual size_t Window_Y() = 0;

    virtual size_t Window_W() = 0;
    virtual size_t Window_H() = 0;

    virtual void InitGame(void* renderer) = 0;

#define DEFINE_DISPATCH_PRSS_FUNC(key) \
    virtual void Dispatch_##key##_Press() {}
#define DEFINE_DISPATCH_RELEASE_FUNC(key) \
    virtual void Dispatch_##key##_Release() {}

    GAME_KEY_LIST_1(DEFINE_DISPATCH_PRSS_FUNC)
    GAME_KEY_LIST_1(DEFINE_DISPATCH_RELEASE_FUNC)

#undef DEFINE_DISPATCH_PRSS_FUNC
#undef DEFINE_DISPATCH_RELEASE_FUNC
};

class GameMgrFactory
{
public:
    static GameMgrFactory& instance();

    std::unique_ptr<GameMgr> CreateMgr(const std::string& game);

    void RegistGameMgr(const std::string& game,
                       const std::function<GameMgr*(void)>& creator)
    {
        EXPECT(!!creator, game);
        creators_.emplace(game, creator);
    }

private:
    std::unordered_map<std::string, std::function<GameMgr*(void)>> creators_;
};

class GameContext
{
public:
    static GameContext& instance();

    ~GameContext();

    bool SwitchGame(const std::string& game);

    GameMgr* CurrentGameMgr()
    {
        return currentMgr_.get();
    }

    template <typename T>
    T* CurrentGameMgr()
    {
        return dynamic_cast<T*>(CurrentGameMgr());
    }

private:
    std::unique_ptr<GameMgr> currentMgr_;
};

struct GameMgrRegistry
{
    GameMgrRegistry(const std::string& game, const std::function<GameMgr*(void)>& creator)
    {
        GameMgrFactory::instance().RegistGameMgr(game, creator);
    }
};

} // namespace games

#define GAME_CONCAT_IMPL(a, b) a##b
#define GAME_CONCAT(a, b) GAME_CONCAT_IMPL(a, b)
#define GAME_UNIQUE_NAME(name) GAME_CONCAT(name, __COUNTER__)

#define REGIST_GAME_MANAGER(name, creator) \
    static games::GameMgrRegistry GAME_UNIQUE_NAME(__game_mgr_registry__)(name, creator);
