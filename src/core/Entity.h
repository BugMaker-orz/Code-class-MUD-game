#pragma once
#include "Position.h"
#include <string>
namespace dq {
class Entity {
public:
    Entity(const std::string& name, const Position& pos, char symbol)
        : m_name(name), m_position(pos), m_symbol(symbol), m_isAlive(true) {}
    virtual ~Entity() = default;
    const std::string& getName() const { return m_name; }
    Position getPosition() const { return m_position; }
    char getSymbol() const { return m_symbol; }
    bool isAlive() const { return m_isAlive; }
    void setPosition(const Position& pos) { m_position = pos; }
    void setAlive(bool alive) { m_isAlive = alive; }
    virtual std::string getDescription() const = 0;
protected:
    std::string m_name;
    Position m_position;
    char m_symbol;
    bool m_isAlive;
};
}
