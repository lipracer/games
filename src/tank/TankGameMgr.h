#pragma once

#include <chrono>
#include <list>
#include <memory>

#include "GameKit/GameMgr.h"
#include "GameKit/Image.h"
#include "GameKit/Timer.h"
#include "GameKit/View.h"
#include "GameMap.h"
#include "TankObject.h"

namespace games
{
namespace tank
{
class EnemyTankManager
{
public:
    struct Config
    {
        size_t speed;
        size_t life_count;
        size_t free_path;
        size_t total;
        size_t max_alive;
        friend std::istream& operator>>(std::istream&, Config& cfg);
    };

    EnemyTankManager();

    /// generate enemy tanks
    void Generate(size_t index);

    void update();

    void reset()
    {
        st_ = std::chrono::steady_clock::now();
        clear();
    }

    void SetPass(size_t p);

    /// load enemy tank's config
    void LoadConfig(std::ifstream& ios);

    void TryGenerate();
    void PrepareGenerate();

    void clear()
    {
        gq_.clear();
        for (auto& l : enemy_tank_)
        {
            l.clear();
        }
    }

private:
    Rect rect_[3];
    Config cfg_[3];

    std::list<SharedObject<Object>> enemy_tank_[3];

    std::list<size_t> gq_;
    decltype(std::chrono::steady_clock::now()) st_ = std::chrono::steady_clock::now();
};

class TankGameMgr : public GameMgr
{
public:
    TankGameMgr();

    ~TankGameMgr() override;

    void DispatchMessage() override;

    void InitGame(void* renderer) override;

    void CreateLeftPlayer();
    void CreateRightPlayer();

    /// device message dispatch
    void Dispatch_LEFT_Press() override;
    void Dispatch_RIGHT_Press() override;
    void Dispatch_UP_Press() override;
    void Dispatch_DOWN_Press() override;
    void Dispatch_A_Press() override;

    void Dispatch_LEFT_Release() override;
    void Dispatch_RIGHT_Release() override;
    void Dispatch_UP_Release() override;
    void Dispatch_DOWN_Release() override;

    void Dispatch_A_Release() override;
    void Dispatch_Q_Press() override;

    void Dispatch_0_Press() override;
    void Dispatch_1_Press() override;
    void Dispatch_2_Press() override;
    void Dispatch_3_Press() override;
    void Dispatch_4_Press() override;

    size_t Window_X() override
    {
        return 0;
    }
    size_t Window_Y() override
    {
        return 0;
    }

    size_t Window_W() override
    {
        return WINDOW_W;
    }
    size_t Window_H() override
    {
        return WINDOW_H;
    }

    void RegistObject(SharedObject<ObjectBase>&& obj);
    void Remove(ObjectBase* obj);

    void RegistOnKeywardEvent(Object* obj, size_t index);

    void SetPass(size_t);

    GameMap& GetMap()
    {
        return map_;
    }
    SharedPtr<Image>* GetEnemyImages()
    {
        return enemy_tank_imgs_;
    }

    SharedPtr<Image> GetBulletImage()
    {
        return bullet_img_;
    };
    SharedPtr<Image> GetBoomImage()
    {
        return boom_img_;
    };

    auto GetTimer10()
    {
        return timer10_.get();
    }
    auto GetTimer20()
    {
        return timer20_.get();
    }
    auto GetTimer100()
    {
        return timer100_.get();
    }
    auto GetTimer500()
    {
        return timer500_.get();
    }
    auto GetTimer1000()
    {
        return timer1000_.get();
    }
    auto GetTimer5000()
    {
        return timer5000_.get();
    }

    void ResetTimer()
    {
        timer10_->reset();
        timer20_->reset();
        timer100_->reset();
        timer500_->reset();
        timer1000_->reset();
        timer5000_->reset();
    }

    void ClearTimer()
    {
        timer10_->clear();
        timer20_->clear();
        timer100_->clear();
        timer500_->clear();
        timer1000_->clear();
        timer5000_->clear();
    }

    void RegistMoveableObject(Object* obj)
    {
        moveable_objs_.push_back(obj);
    }

    void CollsionDetectionBullet();

    bool CollsionDetectionTank(const Rect& r);

    std::list<SharedObject<ObjectBase>>& objs()
    {
        return objs_;
    }

    void GameOver() override;

    void PlayAttack();

    // we need update map after all tank object moved, then we can run collsion detection
    // before next tank move, record location of tank can recude collsion detection
    // algorithm to O1
    void UpdateMap();

    void* renderer_ = nullptr;

    SharedPtr<Image> bakground_img_;

    SharedPtr<Image> home_img_;
    SharedPtr<Image> bad_home_img_;

    SharedPtr<Image> my_tank_img_;
    SharedPtr<Image> bullet_img_;
    SharedPtr<Image> boom_img_;
    SharedPtr<Image> enemy_tank_imgs_[3];

    SharedPtr<Sound> attack_sound_;

#define DEF_MAP_IMAGE_NAME(_) SharedPtr<Image> _##Img_
    DEF_ELEMENT_LIST_0(DEF_MAP_IMAGE_NAME)
#undef DEF_MAP_IMAGE_NAME

    SharedObject<HomeObject> home_obj_;

    SharedObject<MyTank> left_player_;
    SharedObject<MyTank> right_player_;

    std::list<SharedObject<ObjectBase>> objs_;

    std::list<SharedObject<Object>> moveable_objs_;
    SharedObject<Object> key_objs_[10];

    GameMap map_;
    AlivableObjectMap alivable_map_;

    std::function<void(size_t, size_t)>
        draw_functions_[static_cast<size_t>(GameMap::Element::kLast) + 1];

    std::list<std::pair<std::function<void(void)>, int>> listeners_;

    SharedPtr<Timer10> timer10_ = std::make_shared<Timer10>();
    SharedPtr<Timer20> timer20_ = std::make_shared<Timer20>();
    SharedPtr<Timer100> timer100_ = std::make_shared<Timer100>();
    SharedPtr<Timer500> timer500_ = std::make_shared<Timer500>();
    SharedPtr<Timer1000> timer1000_ = std::make_shared<Timer1000>();
    SharedPtr<Timer5000> timer5000_ = std::make_shared<Timer5000>();

    std::unique_ptr<EnemyTankManager> em_;
    bool game_over_ = false;
};
} // namespace tank
} // namespace games

#define GAME_MGR() (*(GameContext::instance().CurrentGameMgr<TankGameMgr>()))
