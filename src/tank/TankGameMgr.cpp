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

    size_t total_size = (MAP_W / 2 - 1) * (MAP_H / 2 - 1) / 3 + 10000;
    do
    {
        gq_.push_back(0);
        gq_.push_back(1);
        gq_.push_back(2);
    } while (total_size--);
}

void EnemyTankManager::Generate(size_t index)
{
    auto t = new EnemyTank(rect_[index]);
    t->SetImage(GAME_MGR().GetEnemyImages()[index])
        ->SetSpeed(cfg_[index].speed)
        ->SetMap(&GAME_MGR().GetMap());
    t->SetFreePath(cfg_[index].free_path)->SetLiftCount(cfg_[index].life_count);
}

void EnemyTankManager::update()
{
    auto cur_st_ = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(cur_st_ - st_).count()
        > 5000)
    {
        st_ = cur_st_;
        for (auto iter = gq_.begin(); iter != gq_.end();)
        {
            bool delay = std::any_of(GAME_MGR().objs().begin(), GAME_MGR().objs().end(),
                                     [&](auto o)
                                     {
                                         if (auto t = o.template cast<TankBase>())
                                         {
                                             if (t->rect().overlap(rect_[*iter]))
                                             {
                                                 return true;
                                             }
                                         }
                                         return false;
                                     });
            if (!delay)
            {
                Generate(*iter);
                iter = gq_.erase(iter);
                continue;
            }
            iter++;
        }
    }
}

void EnemyTankManager::LoadConfig(std::ifstream& ios)
{
    auto GetStringValue = [](const std::string& str)
    { return std::stol(str.data() + str.find(":") + 1); };
    memset(&cfg_, 0, sizeof(cfg_));
    std::string line;
    for (size_t i = 0; i < 3; ++i)
    {
        std::getline(ios, line);
        std::getline(ios, line);
        cfg_[i].speed = GetStringValue(line);
        std::getline(ios, line);
        cfg_[i].life_count = GetStringValue(line);
        std::getline(ios, line);
        cfg_[i].free_path = GetStringValue(line);
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

TankGameMgr::~TankGameMgr() {}

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

    home_img_ = std::make_shared<Image>(renderer, RootResourcePath() + "image/home.bmp");
    bad_home_img_ =
        std::make_shared<Image>(renderer, RootResourcePath() + "image/bad_home.bmp");

    attack_sound_ = std::make_shared<Sound>(RootResourcePath() + "audio/attack.pcm");

    em_.reset(new EnemyTankManager());

    SetPass(0);

    home_obj_ = new HomeObject();
    home_obj_->SetRect(Rect(Point(WINDOW_W / 2, WINDOW_H - ELEMENT_H), TANK_W, TANK_H))
        ->SetImage(home_img_);
    CreateLeftPlayer();
}

void TankGameMgr::PlayAttack()
{
    attack_sound_->play();
}

void TankGameMgr::CreateLeftPlayer()
{
    left_player_ = new MyTank(Rect(0, WINDOW_H - TANK_H, TANK_W, TANK_H));
    left_player_->SetImage(my_tank_img_)
        ->RegistOnKeywardEvent(0)
        ->SetSpeed(5)
        ->SetMap(&map_);

    // right_player_ = new MyTank(Rect(TANK_W, WINDOW_H - TANK_H, TANK_W, TANK_H));
    // right_player_->SetImage(my_tank_img_)
    //     ->RegistOnKeywardEvent(1)
    //     ->SetSpeed(5)
    //     ->SetMap(&map_);
}

void TankGameMgr::CreateRightPlayer() {}

void TankGameMgr::Dispatch_LEFT_Press()
{
    key_objs_[0]->MoveTo_LEFT();
    // key_objs_[1]->MoveTo_LEFT();
}
void TankGameMgr::Dispatch_RIGHT_Press()
{
    key_objs_[0]->MoveTo_RIGHT();
    // key_objs_[1]->MoveTo_RIGHT();
}
void TankGameMgr::Dispatch_UP_Press()
{
    key_objs_[0]->MoveTo_UP();
    // key_objs_[1]->MoveTo_UP();
}
void TankGameMgr::Dispatch_DOWN_Press()
{
    key_objs_[0]->MoveTo_DOWN();
    // key_objs_[1]->MoveTo_DOWN();
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
    em_->clear();
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

void TankGameMgr::CollsionDetection()
{
    for (auto& o0 : moveable_objs_)
    {
        for (auto& o1 : moveable_objs_)
        {
            if (o0 != o1 && o0->alive() && o1->alive())
            {
                if (o0->rect().overlap(o1->rect()))
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
    for (auto o0 : moveable_objs_)
    {
        o0->UpdatePrevRect();
    }
}

void TankGameMgr::GameOver()
{
    home_obj_->SetImage(bad_home_img_);
    for (auto o : objs_)
    {
        o->die();
    }
    timer10_->clear();
    timer20_->clear();
    timer100_->clear();
    timer1000_->clear();

    RemoveDiedObj(moveable_objs_);
    RemoveDiedObj(objs_);
    game_over_ = true;
}

void TankGameMgr::UpdateMap()
{
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
}

void TankGameMgr::DispatchMessage()
{
    if (timer10_->update() || timer20_->update() || timer100_->update()
        || timer1000_->update())
    {
        em_->update();

        RemoveDiedObj(moveable_objs_);

        UpdateMap();

        CollsionDetection();

        RemoveDiedObj(objs_);
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

void TankGameMgr::RegistOnKeywardEvent(Object* obj, size_t index)
{
    key_objs_[index] = obj;
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
}
} // namespace tank
} // namespace games
