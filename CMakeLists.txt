cmake_minimum_required(VERSION 3.15)
project(Code-class-MUD-game-text)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 版本二：纯文字版 MUD
# 不使用二维地图 / 方向键（WASD），全部采用命令式文字输入。
# 二维渲染代码（Renderer / InputManager / FOVSystem）不参与本版本编译。
add_executable(mud_text
    src/text/main_text.cpp
    src/text/TextGame.cpp
    src/core/GameContext.cpp
    src/character/Player.cpp
    src/character/Monster.cpp
    src/character/Npc.cpp
    src/combat/CombatSystem.cpp
    src/map/Map.cpp
    src/map/DungeonGenerator.cpp
    src/item/Item.cpp
    src/item/DropSystem.cpp
    src/item/ShopSystem.cpp
)
target_include_directories(mud_text PRIVATE src)
# MSVC: 强制按 UTF-8 解读源码，避免中文字符串乱码
if(MSVC)
    target_compile_options(mud_text PRIVATE /utf-8)
else()
    # 兼容旧版 MinGW：其 wincon.h 未定义 Windows 10 新增的 VT 转义常量
    # （ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004），这里手动补上，
    # 使 setupConsole() 的 ANSI 彩色输出生效。
    target_compile_definitions(mud_text PRIVATE ENABLE_VIRTUAL_TERMINAL_PROCESSING=0x0004)
endif()
