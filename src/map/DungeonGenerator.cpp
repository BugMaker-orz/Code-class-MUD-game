#include "map/DungeonGenerator.h"
#include <cstdlib>
#include <algorithm>
namespace dq {
Map DungeonGenerator::generateDungeon(int level, int totalLevels) {
    // 每层布局有变化：奇数层横排（3 列 x 2 行），偶数层竖排（2 列 x 3 行）
    // 第 5 层为魔王层，固定横排布局。房间总数保持 6 间（不贪多）。
    if (level % 2 == 0 && level < totalLevels) m_cols = 2;
    else m_cols = 3;
    m_rows = m_config.roomsPerLevel / m_cols;
    if (m_rows < 1) m_rows = 1;
    int W = m_cols * m_config.roomWidth + (m_cols - 1) * m_config.gap;
    int H = m_rows * m_config.roomHeight + (m_rows - 1) * m_config.gap;
    Map map(W, H);
    std::vector<Map::Room> rooms;

    // 1) 生成所有房间矩形（边框为墙，内部为地板）
    for (int gy = 0; gy < m_rows; ++gy) {
        for (int gx = 0; gx < m_cols; ++gx) {
            Map::Room r;
            r.gridX = gx;
            r.gridY = gy;
            int x0 = gx * (m_config.roomWidth + m_config.gap);
            int y0 = gy * (m_config.roomHeight + m_config.gap);
            r.topLeft = Position(x0, y0);
            r.bottomRight = Position(x0 + m_config.roomWidth - 1, y0 + m_config.roomHeight - 1);
            carveRoom(map, r);
            rooms.push_back(r);
            map.addRoom(r);
        }
    }

    // 2) 连接相邻房间（每个房间最多 4 个邻居）
    connectNeighbors(map, rooms);

    // 3) 标注房间属性
    assignTypes(rooms, level, totalLevels);
    for (size_t i = 0; i < rooms.size(); ++i)
        map.setRoomType(i, rooms[i].type);

    return map;
}

void DungeonGenerator::carveRoom(Map& map, const Map::Room& room) {
    // 房间区域初始即墙；把内部挖成地板
    for (int x = room.topLeft.x + 1; x <= room.bottomRight.x - 1; ++x)
        for (int y = room.topLeft.y + 1; y <= room.bottomRight.y - 1; ++y)
            map.setTile(x, y, TileType::Floor);
}

void DungeonGenerator::openDoor(Map& map, const Map::Room& room, char dir) {
    int cx = room.getCenterX();
    int cy = room.getCenterY();
    int x0 = room.topLeft.x, y0 = room.topLeft.y;
    int x1 = room.bottomRight.x, y1 = room.bottomRight.y;
    int gap = m_config.gap;
    // 需挖通 gap 列 + 邻居墙，共 gap+2 格
    if (dir == 'R') {
        for (int d = 0; d <= gap + 1; ++d) map.setTile(x1 + d, cy, TileType::Door);
    } else if (dir == 'L') {
        for (int d = 0; d <= gap + 1; ++d) map.setTile(x0 - d, cy, TileType::Door);
    } else if (dir == 'D') {
        for (int d = 0; d <= gap + 1; ++d) map.setTile(cx, y1 + d, TileType::Door);
    } else if (dir == 'U') {
        for (int d = 0; d <= gap + 1; ++d) map.setTile(cx, y0 - d, TileType::Door);
    }
}

void DungeonGenerator::connectNeighbors(Map& map, std::vector<Map::Room>& rooms) {
    auto indexOf = [&](int gx, int gy) -> int { return gy * m_cols + gx; };
    for (int gy = 0; gy < m_rows; ++gy) {
        for (int gx = 0; gx < m_cols; ++gx) {
            int idx = indexOf(gx, gy);
            // 右侧邻居
            if (gx + 1 < m_cols) {
                int n = indexOf(gx + 1, gy);
                openDoor(map, rooms[idx], 'R');
                map.connectRooms(idx, n);
            }
            // 下方邻居
            if (gy + 1 < m_rows) {
                int n = indexOf(gx, gy + 1);
                openDoor(map, rooms[idx], 'D');
                map.connectRooms(idx, n);
            }
        }
    }
}

void DungeonGenerator::assignTypes(std::vector<Map::Room>& rooms, int level, int totalLevels) {
    // 房间 0（左上角）固定为起点
    rooms[0].type = Map::RoomType::Start;
    // 其余房间随机分配：Boss、楼梯、商店、宝箱、普通
    std::vector<int> rest;
    for (size_t i = 1; i < rooms.size(); ++i) rest.push_back(static_cast<int>(i));
    for (size_t i = rest.size(); i > 1; --i) {
        int j = rand() % static_cast<int>(i);
        std::swap(rest[i - 1], rest[j]);
    }
    size_t p = 0;
    const bool lastLevel = (level >= totalLevels);
    if (lastLevel) {
        // 最后一层：Boss 关，无向下的楼梯
        rooms[rest[p++]].type = Map::RoomType::Boss;
        rooms[rest[p++]].type = Map::RoomType::Shop;
        rooms[rest[p++]].type = Map::RoomType::Treasure;
    } else {
        // 前几层：其中一个房间可通往下一层
        rooms[rest[p++]].type = Map::RoomType::StairsDown;
        rooms[rest[p++]].type = Map::RoomType::Shop;
        rooms[rest[p++]].type = Map::RoomType::Treasure;
    }
    while (p < rest.size())
        rooms[rest[p++]].type = Map::RoomType::Normal;
}
}
