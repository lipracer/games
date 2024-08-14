#include "GameKit/GameMgr.h"

#include "GameKit/GameKit.h"

namespace games
{

GameMgrFactory& GameMgrFactory::instance()
{
    static auto instance = std::make_unique<GameMgrFactory>();
    return *instance;
}

std::unique_ptr<GameMgr> GameMgrFactory::CreateMgr(const std::string& game)
{
    auto iter = creators_.find(game);
    EXPECT(iter != creators_.end(), "expect creator registed");
    return std::unique_ptr<GameMgr>(iter->second());
}

GameContext::~GameContext()
{
    // EXPECT(games::ObjectBase::gMemoryStatistic == 0, "memory leak");
}

GameContext& GameContext::instance()
{
    static auto instance = std::make_unique<GameContext>();
    return *instance;
}

bool GameContext::SwitchGame(const std::string& game)
{
    currentMgr_ = GameMgrFactory::instance().CreateMgr(game);
    return currentMgr_.get();
}

} // namespace games
