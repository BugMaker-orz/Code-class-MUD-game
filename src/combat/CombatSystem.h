#pragma once
#include "core/Position.h"
#include <string>
#include <vector>
namespace dq {
class Player;
class Monster;
class GameContext;
// 单回合战斗结果：双向伤害、击杀标记、经验/金币与可展示消息
struct CombatResult {
    int damageDealt = 0;
    int damageReceived = 0;
    bool targetKilled = false;
    int expGained = 0;
    int goldGained = 0;
    std::string message;
};
// 静态战斗系统：命中判定、伤害计算、玩家/怪物回合结算与死亡掉落。
// 全部为纯函数式静态方法，不持有状态，由 TextGame 调用。
class CombatSystem {
public:
    static CombatResult playerAttackMonster(Player& player, Monster& monster, GameContext& context);
    static CombatResult monsterAttackPlayer(Monster& monster, Player& player, GameContext& context);
    static int calculateDamage(int attackerAttack, int defenderDefense);
    static bool checkHit(int attackerAttack, int defenderDefense);
    static void handleMonsterDeath(Monster& monster, Player& player, GameContext& context);
    static bool isAdjacent(const Position& a, const Position& b);
    static std::vector<Position> getAdjacentPositions(const Position& pos);
};
}
