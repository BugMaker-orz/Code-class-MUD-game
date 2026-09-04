#pragma once
namespace dq {
enum class TileType {
    Wall, Floor, Door, StairsUp, StairsDown, Water, Lava
};
char tileToChar(TileType type); // 由 Renderer 实现
}
