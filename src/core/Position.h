#pragma once
#include <cstdlib>
namespace dq {
// 二维坐标值对象：整型 x/y 与比较、曼哈顿距离等基础运算
struct Position {
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position& other) const { return !(*this == other); }
    int manhattanDistance(const Position& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }
};
}
