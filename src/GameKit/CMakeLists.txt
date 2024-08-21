add_commom_library(GameKit OBJECT
    List.cpp
    Timer.cpp
    Image.cpp
    View.cpp
    Object.cpp
    GameMgr.cpp
)

target_include_directories(GameKit PUBLIC ${CMAKE_SOURCE_DIR}/third_party/SDL/include)
