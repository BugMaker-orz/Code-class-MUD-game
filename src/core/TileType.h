#pragma once
namespace dq {
// 地图瓦片类型：墙/地板/门/上下楼楼梯/水域/岩浆
enum class TileType {
    Wall, Floor, Door, StairsUp, StairsDown, Water, Lava
};
char tileToChar(TileType type); // 由 Renderer 实现
}
