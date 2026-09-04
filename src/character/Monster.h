#pragma once
#include "core/Entity.h"
namespace dq {
// 怪物类型：哥布林/骷髅/蝙蝠/史莱姆/兽人/魔王（数值与掉落各不相同）
enum class MonsterType { Goblin, Skeleton, Bat, Slime, Orc, Boss };
// 怪物实体：属性由"类型 + 层数"初始化，支持受伤与读档恢复血量。
// AIState 字段为与二维版交互预留，纯文字版仅保留敌对标记。
class Monster : public Entity {
public:
    enum class AIState { Idle, Patrol, Chase, Attack };
    Monster(MonsterType type, const Position& pos, int level = 1);
    MonsterType getMonsterType() const { return m_type; }
    int getMaxHp() const { return m_maxHp; }
    int getCurrentHp() const { return m_currentHp; }
    int getAttack() const { return m_attack; }
    int getDefense() const { return m_defense; }
    int getExpReward() const { return m_expReward; }
    int getGoldReward() const { return m_goldReward; }
    bool isHostile() const { return m_hostile; }
    int getViewRadius() const { return m_viewRadius; }
    void takeDamage(int damage);
    void setCurrentHp(int hp);   // 读档恢复剩余血量
    void setHostile(bool hostile) { m_hostile = hostile; }
    AIState getAIState() const { return m_aiState; }
    void setAIState(AIState state) { m_aiState = state; }
    std::string getDescription() const override;
private:
    MonsterType m_type;
    int m_maxHp, m_currentHp;
    int m_attack, m_defense;
    int m_expReward, m_goldReward;
    int m_viewRadius;
    bool m_hostile;
    AIState m_aiState;
    void initializeStats(int level);
};
}
