#include "TankGameMgr.h"

#include <fstream>

#include <string.h>

#include "Bullet.h"
#include "GameKit/GameKit.h"
#include "GameKit/GameMgr.h"

namespace games
{
namespace tank
{

std::istream& operator>>(std::istream& is, EnemyTankManager::Config& cfg)
{
    std::string line;
    std::getline(is, line);
    if (line.find("[tank]") == std::string::npos)
    {
        return is;
    }

    auto GetStringValue = [](const std::string& str)
    { return std::stol(str.data() + str.find("=") + 1); };

#define GET_VALUE(v)        \
    std::getline(is, line); \
    cfg.v = GetStringValue(line);

    GET_VALUE(speed)
    GET_VALUE(life_count)
    GET_VALUE(free_path)
    GET_VALUE(total)
    GET_VALUE(max_alive)

#undef GET_VALUE

    return is;
}

EnemyTankManager::EnemyTankManager()
{
    rect_[0] = Rect(0, 0, TANK_W, TANK_H);
    rect_[1] = Rect(WINDOW_W / 2 - TANK_W / 2, 0, TANK_W, TANK_H);
    rect_[2] = Rect(WINDOW_W - TANK_W, 0, TANK_W, TANK_H);
}

void EnemyTankManager::SetPass(size_t p)
{
    reset();

    Generate(0);
    Generate(1);
    Generate(2);
}

void EnemyTankManager::Generate(size_t index)
{
    auto t = new EnemyTank(rect_[index]);
    t->SetImage(GAME_MGR().GetEnemyImages()[index])
        ->SetWarningImage(GAME_MGR().GetEnemyWarningImages()[index])
        ->SetSpeed(cfg_[index].speed)
        ->SetMap(&GAME_MGR().GetMap());
    t->SetFreePath(cfg_[index].free_path)->SetLiftCount(cfg_[index].life_count);
    --cfg_[index].total;
}

void EnemyTankManager::TryGenerate()
{
    for (auto iter = gq_.begin(); iter != gq_.end();)
    {
        Generate(*iter);
        iter = gq_.erase(iter);
    }
}

void EnemyTankManager::PrepareGenerate()
{
    for (size_t i = 0; i < 3; ++i)
    {
        RemoveDiedObj(enemy_tank_[i]);
        if (cfg_[i].total)
        {
            auto count = enemy_tank_[i].size();
            if (count < cfg_[i].max_alive)
            {
                gq_.push_back(i);
            }
        }
    }
}

void EnemyTankManager::update()
{
    auto cur_st_ = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(cur_st_ - st_).count()
        > 5000)
    {
        st_ = cur_st_;
        TryGenerate();
        PrepareGenerate();
    }
}

void EnemyTankManager::LoadConfig(std::ifstream& ios)
{
    memset(&cfg_, 0, sizeof(cfg_));
    for (size_t i = 0; i < 3; ++i)
    {
        ios >> cfg_[i];
    }
}

static void EmptyDraw(size_t i, size_t j) {}

TankGameMgr::TankGameMgr()
{
#define ASSIGN_DRAW_FUNCTION(_)                                                    \
    draw_functions_[static_cast<size_t>(GameMap::Element::k##_)] =                 \
        [this](size_t i, size_t j)                                                 \
    {                                                                              \
        if (_##Img_)                                                               \
            _##Img_->draw(Rect(ELEMENT_W* i, ELEMENT_W* j, ELEMENT_W, ELEMENT_H)); \
    };
    DEF_ELEMENT_LIST_0(ASSIGN_DRAW_FUNCTION)
#undef ASSIGN_DRAW_FUNCTION
}

TankGameMgr::~TankGameMgr()
{
    Log::info() << "destroy TankGameMgr";
}

#define PI 3.141592652f

static std::string RootResourcePath()
{
    return "./resource/tank/";
}

void TankGameMgr::InitGame(void* renderer)
{
    renderer_ = renderer;
    my_tank_img_ =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/my_up.bmp");

    BrickImg_ = std::make_shared<Image>(renderer, RootResourcePath() + "image/brick.bmp");
    BlockImg_ = std::make_shared<Image>(renderer, RootResourcePath() + "image/block.bmp");

    bullet_img_ =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/bullet.bmp");
    boom_img_ = std::make_shared<Image>(renderer, RootResourcePath() + "image/boom.bmp");

    enemy_tank_imgs_[0] =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/enemy1.bmp");
    enemy_tank_imgs_[1] =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/enemy2.bmp");
    enemy_tank_imgs_[2] =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/enemy3.bmp");

    enemy_tank_warning_imgs_[0] =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/enemy1_red.bmp");
    enemy_tank_warning_imgs_[1] =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/enemy2_red.bmp");
    enemy_tank_warning_imgs_[2] =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/enemy3_red.bmp");

    home_img_ = std::make_shared<Image>(renderer, RootResourcePath() + "image/home.bmp");
    bad_home_img_ =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/bad_home.bmp");

    attack_sound_ = std::make_shared<Sound>(RootResourcePath() + "audio/attack.pcm");

    em_.reset(new EnemyTankManager());

    SetPass(0);

    home_obj_ = new HomeObject();
    home_obj_->SetRect(Rect(Point(WINDOW_W / 2, WINDOW_H - ELEMENT_H), TANK_W, TANK_H))
        ->SetImage(home_img_);
}

void TankGameMgr::PlayAttack()
{
    attack_sound_->play();
}

void TankGameMgr::CreateLeftPlayer()
{
    left_player_ = new MyTank(Rect(0, WINDOW_H - TANK_H, TANK_W, TANK_H));
    left_player_->SetImage(my_tank_img_)->SetSpeed(5)->SetMap(&map_);
    Log::info() << "left_player_.get()->ref_count():" << left_player_.get()->ref_count();
    // EXPECT(left_player_.get()->ref_count() == 4, "");

    // right_player_ = new MyTank(Rect(TANK_W, WINDOW_H - TANK_H, TANK_W, TANK_H));
    // right_player_->SetImage(my_tank_img_)
    //     ->SetSpeed(5)
    //     ->SetMap(&map_);
}

void TankGameMgr::CreateRightPlayer() {}

void TankGameMgr::Dispatch_LEFT_Press()
{
    if (!game_over_)
        left_player_->MoveTo_LEFT();
    // right_player_->MoveTo_LEFT();
}

void TankGameMgr::Dispatch_RIGHT_Press()
{
    if (!game_over_)
        left_player_->MoveTo_RIGHT();
    // right_player_->MoveTo_RIGHT();
}

void TankGameMgr::Dispatch_UP_Press()
{
    if (!game_over_)
        left_player_->MoveTo_UP();
    // right_player_->MoveTo_UP();
}

void TankGameMgr::Dispatch_DOWN_Press()
{
    if (!game_over_)
        left_player_->MoveTo_DOWN();
    // right_player_->MoveTo_DOWN();
}

void TankGameMgr::Dispatch_LEFT_Release() {}
void TankGameMgr::Dispatch_RIGHT_Release() {}
void TankGameMgr::Dispatch_UP_Release() {}
void TankGameMgr::Dispatch_DOWN_Release() {}

void TankGameMgr::Dispatch_A_Press()
{
    left_player_->OnAttackPress();
}
void TankGameMgr::Dispatch_A_Release()
{
    left_player_->OnAttackRelease();
}

void TankGameMgr::Dispatch_Q_Press()
{
    GameOver();
}
void TankGameMgr::Dispatch_0_Press()
{
    SetPass(0);
}
void TankGameMgr::Dispatch_1_Press()
{
    SetPass(0);
}
void TankGameMgr::Dispatch_2_Press()
{
    SetPass(0);
}
void TankGameMgr::Dispatch_3_Press()
{
    SetPass(0);
}
void TankGameMgr::Dispatch_4_Press()
{
    SetPass(0);
}

void TankGameMgr::RegistObject(SharedObject<ObjectBase>&& obj)
{
    objs_.push_front(std::move(obj));
}

void TankGameMgr::CollsionDetectionBullet()
{
    for (auto& o0 : moveable_objs_)
    {
        for (auto& o1 : moveable_objs_)
        {
            if (o0 != o1 && o0->alive() && o1->alive() && o0->rect().overlap(o1->rect()))
            {
                if (auto b = o0.cast<Bullet>())
                {
                    if (b->MeetToDie(o1.get()))
                    {
                        continue;
                    }
                }
                else if (auto b = o1.cast<Bullet>())
                {
                    if (b->MeetToDie(o0.get()))
                    {
                        continue;
                    }
                }
            }
        }
    }
}

bool TankGameMgr::CollsionDetectionTank(const Rect& r)
{
    return alivable_map_.CollsionDetection(r);
}

void TankGameMgr::DestroyHome()
{
    home_obj_->SetImage(bad_home_img_);
}

void TankGameMgr::GameOver()
{
    if (!game_over_)
    {
        game_over_ = true;
        DestroyAll();
    }
}

void TankGameMgr::UpdateMap()
{
    // clear died tank because collsion detection will product died object
    for (auto& o : objs_)
    {
        if (auto tank = dynamic_cast<TankBase*>(o.get()))
        {
            if (!tank->alive())
            {
                // collsion detection only detection current location, but the
                // alivable_map didn't update now, so we need clear previous rect
                alivable_map_.ClearMap(tank, tank->PrevRect());
            }
        }
    }

    RemoveDiedObj(objs_);

    // first collsion detection
    for (auto& o : objs_)
    {
        if (auto tank = dynamic_cast<TankBase*>(o.get()))
        {
            alivable_map_.ClearMap(tank, tank->PrevRect());
            if (alivable_map_.CollsionDetection(tank->rect()))
            {
                tank->SetRect(tank->PrevRect());
                tank->SetMoveable(false);
            }
            alivable_map_.MaskMap(tank, tank->rect());
        }
    }

    // second collsion detection, after first collsion some object alread release
    // position, therefore we need to constantly detect  objects that  can move until all
    // objects cannot move
    for (auto& o : objs_)
    {
        if (auto tank = dynamic_cast<TankBase*>(o.get()))
        {
            alivable_map_.ClearMap(tank, tank->rect());
            if (tank->Moveable() && !alivable_map_.CollsionDetection(tank->NextRect()))
            {
                tank->SetRect(tank->NextRect());
            }
            alivable_map_.MaskMap(tank, tank->rect());
        }
    }

    for (auto& o : objs_)
    {
        if (auto ob = dynamic_cast<Object*>(o.get()))
        {
            ob->UpdatePrevRect();
        }
    }
    // std::cout << alivable_map_.to_string() << std::endl;
}

void TankGameMgr::DispatchMessage()
{
    while (!message_.empty())
    {
        auto msg = message_.front();
        message_.pop();
        if (msg == GameMessage::kGameOver)
        {
            GameOver();
        }
    }
    if (game_over_)
    {
        return;
    }
    if (timer10_->update() || timer20_->update() || timer100_->update()
        || timer500_->update() || timer1000_->update())
    {
        em_->update();

        RemoveDiedObj(moveable_objs_);

        CollsionDetectionBullet();

        UpdateMap();
    }
    for (auto& o : objs_)
    {
        EXPECT(o, "object can't be null!");
        o->draw();
    }
    for (size_t i = 1; i < MAP_H - 1; ++i)
    {
        for (size_t j = 1; j < MAP_W - 1; ++j)
        {
            auto e = static_cast<size_t>(map_[i][j]);
            draw_functions_[e](j - 1, i - 1);
        }
    }
}

void TankGameMgr::SetPass(size_t pass)
{
    std::ifstream ifs(RootResourcePath() + "pass/pass_0.txt");
    em_->LoadConfig(ifs);
    map_.LoadMap(ifs);
    map_[MAP_H - 2][MAP_W / 2 - 1] = GameMap::Element::kHome;
    map_[MAP_H - 2][MAP_W / 2] = GameMap::Element::kHome;
    map_[MAP_H - 3][MAP_W / 2 - 1] = GameMap::Element::kHome;
    map_[MAP_H - 3][MAP_W / 2] = GameMap::Element::kHome;

    game_over_ = false;
    ResetTimer();

    em_->SetPass(pass);
    CreateLeftPlayer();
}

void TankGameMgr::DestroyAll()
{
    Log::info() << "release mgr object";

    // if (home_obj_)
    //     home_obj_.release();

    Log::info() << "release left_player object";
    if (left_player_)
        left_player_.reset();
    // if (right_player_)
    //     right_player_.release();

    Log::info() << "clear objs_";
    objs_.clear();

    Log::info() << "clear moveable_objs_";
    moveable_objs_.clear();

    Log::info() << "clear timer object";
    ClearTimer();
}

} // namespace tank
} // namespace games
