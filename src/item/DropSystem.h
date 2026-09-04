#pragma once
#include "core/Position.h"
#include "item/Item.h"
#include <vector>
namespace dq {
class Monster;
class GameContext;
struct DropEntry {
    ItemCategory category;
    std::string name;
    char symbol;
    float probability;
    int minCount, maxCount;
    int value;          // 用于金币等
    int attackBonus;    // 用于武器
    int defenseBonus;   // 用于护甲
    int healAmount;     // 用于药水
    ItemRarity rarity = ItemRarity::Common;
};
class DropSystem {
public:
    static std::vector<Item*> generateDrops(const Monster& monster, const Position& pos, GameContext& context);
    static std::vector<Item*> generateFromTable(const std::vector<DropEntry>& table, const Position& pos);
    static std::vector<DropEntry> getGoblinDropTable();
    static std::vector<DropEntry> getSkeletonDropTable();
    static std::vector<DropEntry> getBatDropTable();
    static std::vector<DropEntry> getSlimeDropTable();
    static std::vector<DropEntry> getOrcDropTable();
    static std::vector<DropEntry> getBossDropTable();
    static Item* generateRandomWeapon(int level);
    static Item* generateRandomArmor(int level);
    static Item* generateRandomPotion();
    static Item* generateRandomItem(int level);
private:
    static bool rollProbability(float prob);
    static int randomInt(int min, int max);
};
}
