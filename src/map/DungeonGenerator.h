#pragma once
#include "map/Map.h"
namespace dq {
// 元气骑士式地牢生成器：
// 每层在网格上排列少量房间（默认 6 间），且每层布局有变化：
// 奇数层横排（3 列 x 2 行），偶数层竖排（2 列 x 3 行），第 5 层为魔王层横排。
// 每个房间最多与上下左右 4 个邻居相连，房间均被标注属性
// （起点/商店/宝箱/Boss/楼梯/普通），其中楼梯房间可通往下一层。
class DungeonGenerator {
public:
    struct Config {
        int roomsPerLevel = 6;   // 每层房间总数（排成矩形网格）
        int roomWidth = 13;      // 单个房间含墙的宽度（内部 11 格，更宽敞）
        int roomHeight = 9;      // 单个房间含墙的高度（内部 7 格）
        int gap = 1;             // 房间之间的墙间隔
    };
    DungeonGenerator() = default;
    explicit DungeonGenerator(const Config& config) : m_config(config) {}
    // level 从 1 开始；totalLevels 为总层数
    Map generateDungeon(int level, int totalLevels);
private:
    Config m_config;
    int m_cols = 3;
    int m_rows = 2;
    void carveRoom(Map& map, const Map::Room& room);
    void openDoor(Map& map, const Map::Room& room, char dir);  // dir: 'L','R','U','D'
    void connectNeighbors(Map& map, std::vector<Map::Room>& rooms);
    void assignTypes(std::vector<Map::Room>& rooms, int level, int totalLevels);
};
}
