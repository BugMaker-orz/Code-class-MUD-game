#pragma once
#include "core/TileType.h"
#include "core/Position.h"
#include <vector>
namespace dq {
// 地图类：二维瓦片网格 + 房间列表。纯文字版仍用房间拓扑做小地图与
// 移动判定，瓦片级数据保留以便与二维渲染共用（不参与本版编译）。
class Map {
public:
    // 房间属性：起点/普通/Boss/宝箱/商店/楼梯/走廊
    enum class RoomType { Normal, Start, Boss, Treasure, Shop, StairsDown, Corridor };
    Map(int width, int height);
    Map(int width, int height, const std::vector<std::vector<TileType>>& data);
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    TileType getTile(int x, int y) const;
    TileType getTile(const Position& pos) const;
    void setTile(int x, int y, TileType type);
    void setTile(const Position& pos, TileType type);
    bool isValidPosition(int x, int y) const;
    bool isValidPosition(const Position& pos) const;
    bool isWalkable(int x, int y) const;
    bool isWalkable(const Position& pos) const;
    bool isVisible(int x, int y) const;
    bool isVisible(const Position& pos) const;
    void setVisible(int x, int y, bool visible);
    void setVisible(const Position& pos, bool visible);
    bool isExplored(int x, int y) const;
    bool isExplored(const Position& pos) const;
    void setExplored(int x, int y, bool explored);
    void setExplored(const Position& pos, bool explored);
    // 房间结构体：矩形范围、地板格、属性类型、连通邻居下标与网格坐标
    struct Room {
        Position topLeft, bottomRight;
        std::vector<Position> floorTiles;
        RoomType type = RoomType::Normal;
        std::vector<int> connectedRooms;
        int gridX = -1, gridY = -1;   // 房间在关卡网格中的坐标（用于小地图）
        int getCenterX() const { return (topLeft.x + bottomRight.x) / 2; }
        int getCenterY() const { return (topLeft.y + bottomRight.y) / 2; }
        Position getCenter() const { return Position(getCenterX(), getCenterY()); }
        bool contains(const Position& pos) const {
            return pos.x >= topLeft.x && pos.x <= bottomRight.x &&
                   pos.y >= topLeft.y && pos.y <= bottomRight.y;
        }
    };
    const std::vector<Room>& getRooms() const { return m_rooms; }
    void addRoom(const Room& room) { m_rooms.push_back(room); }
    size_t getRoomCount() const { return m_rooms.size(); }
    const Room* getRoom(size_t index) const;
    void connectRooms(size_t roomIndex1, size_t roomIndex2);
    void setRoomType(size_t roomIndex, RoomType type);
private:
    int m_width, m_height;
    std::vector<std::vector<TileType>> m_tiles;
    std::vector<std::vector<bool>> m_explored;
    std::vector<std::vector<bool>> m_visible;
    std::vector<Room> m_rooms;
};
}
