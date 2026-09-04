#pragma once
#include "core/Position.h"
#include <string>
#include <vector>
namespace dq {
class Player;
class Monster;
class GameContext;
struct CombatResult {
    int damageDealt = 0;
    int damageReceived = 0;
    bool targetKilled = false;
    int expGained = 0;
    int goldGained = 0;
    std::string message;
};
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
