#pragma once
#include "core/Entity.h"
namespace dq {
enum class MonsterType { Goblin, Skeleton, Bat, Slime, Orc, Boss };
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
