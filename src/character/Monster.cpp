#include "character/Monster.h"
namespace dq {
// 构造函数：按类型设置中文名与显示符号，再按层数初始化属性
Monster::Monster(MonsterType type, const Position& pos, int level)
    : Entity("", pos, 'g'), m_type(type), m_hostile(true), m_aiState(AIState::Idle) {
    // 怪物类型 → 中文名/显示符号映射
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
// 按类型 + 层数计算属性：血量/攻防/经验/金币随层数线性成长，
// 定位差异明显——蝙蝠快而脆、兽人血厚、魔王为最高数值的 Boss
void Monster::initializeStats(int level) {
    switch (m_type) {
        case MonsterType::Goblin: m_maxHp=10+level*2; m_attack=3+level; m_defense=1; m_expReward=10+level*2; m_goldReward=5+level; m_viewRadius=4; break;
        case MonsterType::Skeleton: m_maxHp=15+level*3; m_attack=5+level; m_defense=3; m_expReward=15+level*3; m_goldReward=8+level; m_viewRadius=5; break;
        case MonsterType::Bat: m_maxHp=5+level; m_attack=2+level; m_defense=0; m_expReward=5+level; m_goldReward=1; m_viewRadius=5; break;
        case MonsterType::Slime: m_maxHp=8+level*2; m_attack=1+level; m_defense=0; m_expReward=5+level; m_goldReward=2; m_viewRadius=3; break;
        case MonsterType::Orc: m_maxHp=25+level*4; m_attack=8+level; m_defense=5; m_expReward=30+level*5; m_goldReward=15+level; m_viewRadius=5; break;
        case MonsterType::Boss: m_maxHp=60+level*10; m_attack=10+level; m_defense=7; m_expReward=100+level*10; m_goldReward=50+level; m_viewRadius=7; break;
    }
    m_currentHp = m_maxHp;
}
// 受伤结算：生命下限 0，归零即标记死亡
void Monster::takeDamage(int damage) {
    m_currentHp -= damage;
    if (m_currentHp < 0) m_currentHp = 0;
    if (m_currentHp == 0) m_isAlive = false;
}
// 读档恢复剩余血量：不超上限，归零判定死亡
void Monster::setCurrentHp(int hp) {
    m_currentHp = hp;
    if (m_currentHp > m_maxHp) m_currentHp = m_maxHp;
    if (m_currentHp <= 0) { m_currentHp = 0; m_isAlive = false; }
}
// 描述文本：状态栏/战斗信息复用，显示当前/满血
std::string Monster::getDescription() const {
    return m_name + "（生命 " + std::to_string(m_currentHp) + "/" + std::to_string(m_maxHp) + "）";
}
}
