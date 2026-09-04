#include "character/Monster.h"
namespace dq {
Monster::Monster(MonsterType type, const Position& pos, int level)
    : Entity("", pos, 'g'), m_type(type), m_hostile(true), m_aiState(AIState::Idle) {
    switch (type) {
        case MonsterType::Goblin: m_name = "哥布林"; m_symbol = 'g'; break;
        case MonsterType::Skeleton: m_name = "骷髅"; m_symbol = 's'; break;
        case MonsterType::Bat: m_name = "蝙蝠"; m_symbol = 'b'; break;
        case MonsterType::Slime: m_name = "史莱姆"; m_symbol = 'm'; break;
        case MonsterType::Orc: m_name = "兽人"; m_symbol = 'o'; break;
        case MonsterType::Boss: m_name = "地牢魔王"; m_symbol = 'B'; break;
    }
    initializeStats(level);
}
void Monster::initializeStats(int level) {
    switch (m_type) {
        case MonsterType::Goblin: m_maxHp=10+level*2; m_attack=3+level; m_defense=1; m_expReward=10+level*2; m_goldReward=5+level; m_viewRadius=4; break;
        case MonsterType::Skeleton: m_maxHp=15+level*3; m_attack=5+level; m_defense=3; m_expReward=15+level*3; m_goldReward=8+level; m_viewRadius=5; break;
        case MonsterType::Bat: m_maxHp=5+level; m_attack=2+level; m_defense=0; m_expReward=5+level; m_goldReward=1; m_viewRadius=5; break;
        case MonsterType::Slime: m_maxHp=8+level*2; m_attack=1+level; m_defense=0; m_expReward=5+level; m_goldReward=2; m_viewRadius=3; break;
        case MonsterType::Orc: m_maxHp=25+level*4; m_attack=8+level; m_defense=5; m_expReward=30+level*5; m_goldReward=15+level; m_viewRadius=5; break;
        case MonsterType::Boss: m_maxHp=60+level*10; m_attack=12+level; m_defense=8; m_expReward=100+level*10; m_goldReward=50+level; m_viewRadius=7; break;
    }
    m_currentHp = m_maxHp;
}
void Monster::takeDamage(int damage) {
    m_currentHp -= damage;
    if (m_currentHp < 0) m_currentHp = 0;
    if (m_currentHp == 0) m_isAlive = false;
}
void Monster::setCurrentHp(int hp) {
    m_currentHp = hp;
    if (m_currentHp > m_maxHp) m_currentHp = m_maxHp;
    if (m_currentHp <= 0) { m_currentHp = 0; m_isAlive = false; }
}
std::string Monster::getDescription() const {
    return m_name + "（生命 " + std::to_string(m_currentHp) + "/" + std::to_string(m_maxHp) + "）";
}
}
