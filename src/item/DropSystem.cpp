#include "item/DropSystem.h"
#include "character/Monster.h"
#include "core/GameContext.h"
#include <cstdlib>
namespace dq {
// 入口：按怪物类型选择对应掉落表，再逐条掷骰生成
std::vector<Item*> DropSystem::generateDrops(const Monster& monster, const Position& pos, GameContext& context) {
    std::vector<DropEntry> table;
    switch (monster.getMonsterType()) {
        case MonsterType::Goblin: table = getGoblinDropTable(); break;
        case MonsterType::Skeleton: table = getSkeletonDropTable(); break;
        case MonsterType::Bat: table = getBatDropTable(); break;
        case MonsterType::Slime: table = getSlimeDropTable(); break;
        case MonsterType::Orc: table = getOrcDropTable(); break;
        case MonsterType::Boss: table = getBossDropTable(); break;
    }
    return generateFromTable(table, pos);
}
// 逐条掉落表掷骰：命中概率则随机数量生成物品，全部落在指定位置
std::vector<Item*> DropSystem::generateFromTable(const std::vector<DropEntry>& table, const Position& pos) {
    std::vector<Item*> drops;
    for (const auto& entry : table) {
        if (rollProbability(entry.probability)) {
            int count = randomInt(entry.minCount, entry.maxCount);
            for (int i = 0; i < count; ++i) {
                Item* item = new Item(entry.name, entry.symbol, entry.category, entry.rarity);
                item->setPosition(pos);
                item->setValue(entry.value);
                item->setAttackBonus(entry.attackBonus);
                item->setDefenseBonus(entry.defenseBonus);
                item->setHealAmount(entry.healAmount);
                drops.push_back(item);
            }
        }
    }
    return drops;
}
// 掉落表（怪物 → 战利品池）：金币 60%，治疗药水 20%
std::vector<DropEntry> DropSystem::getGoblinDropTable() {
    return {
        {ItemCategory::Gold, "金币", '$', 0.6f, 1, 4, 1, 0, 0, 0, ItemRarity::Common},
        {ItemCategory::Potion, "治疗药水", '!', 0.2f, 1, 1, 10, 0, 0, 8, ItemRarity::Common}
    };
}
// 骷髅：金币 80%，低概率掉武器/护甲
std::vector<DropEntry> DropSystem::getSkeletonDropTable() {
    return {
        {ItemCategory::Gold, "金币", '$', 0.8f, 2, 6, 1, 0, 0, 0, ItemRarity::Common},
        {ItemCategory::Weapon, "锈铁剑", '/', 0.2f, 1, 1, 25, 2, 0, 0, ItemRarity::Uncommon},
        {ItemCategory::Armor, "旧皮甲", '[', 0.1f, 1, 1, 25, 0, 1, 0, ItemRarity::Uncommon}
    };
}
// 蝙蝠：穷怪，仅金币 40%
std::vector<DropEntry> DropSystem::getBatDropTable() {
    return {
        {ItemCategory::Gold, "金币", '$', 0.4f, 1, 2, 1, 0, 0, 0, ItemRarity::Common}
    };
}
// 史莱姆：金币 30%，药水 35%，野果 20%
std::vector<DropEntry> DropSystem::getSlimeDropTable() {
    return {
        {ItemCategory::Gold, "金币", '$', 0.3f, 1, 3, 1, 0, 0, 0, ItemRarity::Common},
        {ItemCategory::Potion, "治疗药水", '!', 0.35f, 1, 1, 10, 0, 0, 10, ItemRarity::Common},
        {ItemCategory::Food, "野果", '%', 0.2f, 1, 1, 5, 0, 0, 5, ItemRarity::Common}
    };
}
// 兽人：高金币，30% 掉稀有战斧、25% 掉兽皮甲
std::vector<DropEntry> DropSystem::getOrcDropTable() {
    return {
        {ItemCategory::Gold, "金币", '$', 0.9f, 5, 12, 1, 0, 0, 0, ItemRarity::Common},
        {ItemCategory::Weapon, "兽人战斧", '/', 0.3f, 1, 1, 60, 4, 0, 0, ItemRarity::Rare},
        {ItemCategory::Armor, "兽皮甲", '[', 0.25f, 1, 1, 55, 0, 3, 0, ItemRarity::Rare}
    };
}
// 魔王：必掉大量金币，高概率掉史诗武器/铠甲
std::vector<DropEntry> DropSystem::getBossDropTable() {
    return {
        {ItemCategory::Gold, "金币", '$', 1.0f, 25, 60, 1, 0, 0, 0, ItemRarity::Common},
        {ItemCategory::Potion, "大治疗药水", '!', 0.9f, 1, 2, 40, 0, 0, 25, ItemRarity::Rare},
        {ItemCategory::Weapon, "魔法剑", '/', 0.6f, 1, 1, 120, 6, 0, 0, ItemRarity::Epic},
        {ItemCategory::Armor, "秘银铠甲", '[', 0.5f, 1, 1, 120, 0, 5, 0, ItemRarity::Epic}
    };
}
// 随机武器：品质随层数提升，攻击加成随层数浮动
Item* DropSystem::generateRandomWeapon(int level) {
    const char* names[] = {"铁剑", "长剑", "战斧", "精钢剑"};
    Item* weapon = new Item(names[rand() % 4], '/', ItemCategory::Weapon,
                            level > 3 ? ItemRarity::Rare : ItemRarity::Uncommon);
    weapon->setAttackBonus(level + rand() % 3);
    weapon->setValue(20 + level * 5);
    return weapon;
}
// 随机护甲：品质随层数提升，防御加成随层数浮动
Item* DropSystem::generateRandomArmor(int level) {
    const char* names[] = {"皮甲", "锁子甲", "铁甲", "鳞甲"};
    Item* armor = new Item(names[rand() % 4], '[', ItemCategory::Armor,
                           level > 3 ? ItemRarity::Rare : ItemRarity::Uncommon);
    armor->setDefenseBonus(level + rand() % 2);
    armor->setValue(20 + level * 5);
    return armor;
}
Item* DropSystem::generateRandomPotion() {
    Item* potion = new Item("治疗药水", '!', ItemCategory::Potion);
    potion->setHealAmount(10);
    potion->setValue(10);
    return potion;
}
Item* DropSystem::generateRandomItem(int level) {
    int r = rand() % 3;
    if (r == 0) return generateRandomWeapon(level);
    if (r == 1) return generateRandomArmor(level);
    return generateRandomPotion();
}
bool DropSystem::rollProbability(float prob) {
    return (rand() % 100) / 100.0f < prob;
}
int DropSystem::randomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}
}
