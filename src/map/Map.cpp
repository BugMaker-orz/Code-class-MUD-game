#include "map/Map.h"
namespace dq {
Map::Map(int width, int height)
    : m_width(width), m_height(height),
      m_tiles(height, std::vector<TileType>(width, TileType::Wall)),
      m_explored(height, std::vector<bool>(width, false)),
      m_visible(height, std::vector<bool>(width, false)) {}
Map::Map(int width, int height, const std::vector<std::vector<TileType>>& data)
    : m_width(width), m_height(height), m_tiles(data),
      m_explored(height, std::vector<bool>(width, false)),
      m_visible(height, std::vector<bool>(width, false)) {}
TileType Map::getTile(int x, int y) const {
    if (!isValidPosition(x, y)) return TileType::Wall;
    return m_tiles[y][x];
}
TileType Map::getTile(const Position& pos) const {
    return getTile(pos.x, pos.y);
}
void Map::setTile(int x, int y, TileType type) {
    if (isValidPosition(x, y)) m_tiles[y][x] = type;
}
void Map::setTile(const Position& pos, TileType type) {
    setTile(pos.x, pos.y, type);
}
bool Map::isValidPosition(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}
bool Map::isValidPosition(const Position& pos) const {
    return isValidPosition(pos.x, pos.y);
}
bool Map::isWalkable(int x, int y) const {
    if (!isValidPosition(x, y)) return false;
    TileType t = m_tiles[y][x];
    return t == TileType::Floor || t == TileType::Door ||
           t == TileType::StairsUp || t == TileType::StairsDown;
}
bool Map::isWalkable(const Position& pos) const {
    return isWalkable(pos.x, pos.y);
}
bool Map::isVisible(int x, int y) const {
    if (!isValidPosition(x, y)) return false;
    return m_visible[y][x];
}
bool Map::isVisible(const Position& pos) const {
    return isVisible(pos.x, pos.y);
}
void Map::setVisible(int x, int y, bool visible) {
    if (isValidPosition(x, y)) m_visible[y][x] = visible;
}
void Map::setVisible(const Position& pos, bool visible) {
    setVisible(pos.x, pos.y, visible);
}
bool Map::isExplored(int x, int y) const {
    if (!isValidPosition(x, y)) return false;
    return m_explored[y][x];
}
bool Map::isExplored(const Position& pos) const {
    return isExplored(pos.x, pos.y);
}
void Map::setExplored(int x, int y, bool explored) {
    if (isValidPosition(x, y)) m_explored[y][x] = explored;
}
void Map::setExplored(const Position& pos, bool explored) {
    setExplored(pos.x, pos.y, explored);
}
const Map::Room* Map::getRoom(size_t index) const {
    if (index < m_rooms.size()) return &m_rooms[index];
    return nullptr;
}
void Map::connectRooms(size_t roomIndex1, size_t roomIndex2) {
    if (roomIndex1 < m_rooms.size() && roomIndex2 < m_rooms.size()) {
        m_rooms[roomIndex1].connectedRooms.push_back(roomIndex2);
        m_rooms[roomIndex2].connectedRooms.push_back(roomIndex1);
    }
}
void Map::setRoomType(size_t roomIndex, RoomType type) {
    if (roomIndex < m_rooms.size()) {
        m_rooms[roomIndex].type = type;
    }
}
}
