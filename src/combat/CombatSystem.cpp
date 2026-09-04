#include "combat/CombatSystem.h"
#include "character/Player.h"
#include "character/Monster.h"
#include "core/GameContext.h"
#include "item/DropSystem.h"
#include <cstdlib>
namespace dq {
bool CombatSystem::checkHit(int attackerAttack, int defenderDefense) {
    int hitChance = 70 + attackerAttack * 2 - defenderDefense * 2;
    if (hitChance > 95) hitChance = 95;
    if (hitChance < 20) hitChance = 20;
    return (rand() % 100) < hitChance;
}
int CombatSystem::calculateDamage(int attackerAttack, int defenderDefense) {
    int base = attackerAttack - defenderDefense / 2;
    if (base < 1) base = 1;
    return base + (rand() % 3);
}
CombatResult CombatSystem::playerAttackMonster(Player& player, Monster& monster, GameContext& context) {
    CombatResult result;
    if (!checkHit(player.getAttack(), monster.getDefense())) {
        result.message = "你挥剑攻击" + monster.getName() + "，可惜落空了！（"
            + monster.getName() + " HP " + std::to_string(monster.getCurrentHp()) + "/"
            + std::to_string(monster.getMaxHp()) + "）";
        return result;
    }
    int damage = calculateDamage(player.getAttack(), monster.getDefense());
    monster.takeDamage(damage);
    result.damageDealt = damage;
    result.message = "你攻击" + monster.getName() + "，造成 " + std::to_string(damage) + " 点伤害！";
    if (!monster.isAlive()) {
        result.targetKilled = true;
        result.expGained = monster.getExpReward();
        result.goldGained = monster.getGoldReward();
        handleMonsterDeath(monster, player, context);
        result.message += " " + monster.getName() + " 被击败了！获得 "
            + std::to_string(result.expGained) + " 经验，"
            + std::to_string(result.goldGained) + " 金币。";
    } else {
        // 未击杀：像向导那样把剩余血量标出来
        result.message += "（" + monster.getName() + " HP "
            + std::to_string(monster.getCurrentHp()) + "/"
            + std::to_string(monster.getMaxHp()) + "）";
    }
    return result;
}
CombatResult CombatSystem::monsterAttackPlayer(Monster& monster, Player& player, GameContext& context) {
    CombatResult result;
    if (!checkHit(monster.getAttack(), player.getDefense())) {
        result.message = monster.getName() + " 向你扑来，但没有打中你。（"
            + monster.getName() + " HP " + std::to_string(monster.getCurrentHp()) + "/"
            + std::to_string(monster.getMaxHp()) + "）";
        return result;
    }
    int damage = calculateDamage(monster.getAttack(), player.getDefense());
    player.takeDamage(damage);
    result.damageReceived = damage;
    result.message = monster.getName() + " 攻击了你，造成 " + std::to_string(damage) + " 点伤害！（"
        + monster.getName() + " HP " + std::to_string(monster.getCurrentHp()) + "/"
        + std::to_string(monster.getMaxHp()) + "）";
    if (!player.isAlive()) {
        result.targetKilled = true;
        context.state = GameContext::GameState::Dead;
    }
    return result;
}
void CombatSystem::handleMonsterDeath(Monster& monster, Player& player, GameContext& context) {
    player.gainExp(monster.getExpReward());
    player.addGold(monster.getGoldReward());
    auto drops = DropSystem::generateDrops(monster, monster.getPosition(), context);
    for (auto item : drops) {
        context.addItemOnGround(item);
    }
    // 从列表中移除；对象所有权由 GameLoop 负责 delete
    context.removeMonster(&monster);
}
bool CombatSystem::isAdjacent(const Position& a, const Position& b) {
    return a.manhattanDistance(b) <= 1;
}
std::vector<Position> CombatSystem::getAdjacentPositions(const Position& pos) {
    std::vector<Position> positions;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (dx != 0 || dy != 0) positions.push_back(Position(pos.x + dx, pos.y + dy));
    return positions;
}
}
