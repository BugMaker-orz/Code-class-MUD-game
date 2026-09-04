# 纯文字版地牢 MUD（C++ 程序设计实验）

C++17 终端文字地牢游戏（MUD）。纯命令行交互，无图形界面，通过输入 `去 东`、`攻击 史莱姆`、`开宝箱` 等命令游玩。

## 玩法

- 5 层地牢，每层 6 个房间（奇数层横排、偶数层竖排），房间标注起点 / 战斗 / 商店 / 宝箱 / 楼梯 / 魔王
- 回合制战斗、商店交易、宝箱拾取、NPC 剧情对话
- **双结局**：正常击败魔王 → 勇者结局；击杀向导夺取资源后击败魔王 → 新魔王结局
- 全视野，每帧显示：小地图 → 房间描述 → 信息 → 状态
- ANSI 彩色界面，UTF-8 中文

## 构建与运行

```bash
mkdir build && cd build
cmake ..
cmake --build .
./mud_text            # 随机地图
./mud_text 42         # 固定种子，地图可复现
```

要求：C++17 编译器（g++ / clang++ / MSVC）+ CMake ≥ 3.15。

## 目录结构

```
src/
├── text/          # 文字版专属：main_text.cpp（入口）、TextGame.h/.cpp（主控类）
├── core/          # 基础层：GameContext（全局状态）、Entity（实体基类）、Position、TileType
├── character/     # Player、Monster、Npc
├── combat/        # CombatSystem（回合制战斗结算）
├── map/           # Map（瓦片+房间）、DungeonGenerator（地牢生成）
└── item/          # Item、DropSystem（掉落）、ShopSystem（商店）
```

## 文档

- [技术文档.md](技术文档.md)：C++ 程序设计实验技术文档（架构设计、核心类、算法、踩坑记录）
