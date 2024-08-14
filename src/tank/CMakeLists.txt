add_commom_library(Tank STATIC 
    TankObject.cpp
    GameMap.cpp
    TankGameMgr.cpp
    Bullet.cpp
    Animation.cpp
)

target_include_directories(Tank PRIVATE ${CMAEK_CURRENT_SRC_DIR})
