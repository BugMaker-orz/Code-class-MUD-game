#pragma once
#include "core/Entity.h"
#include <string>
namespace dq {
class Player;
// 物品类别：武器/护甲可装备，药水/食物回血，金币/钥匙/任务物品等
enum class ItemCategory { Weapon, Armor, Potion, Scroll, Gold, Key, Food, QuestItem };
// 稀有度：决定显示颜色与掉落权重
enum class ItemRarity { Common, Uncommon, Rare, Epic, Legendary };

std::string categoryToChinese(ItemCategory category);
std::string rarityToChinese(ItemRarity rarity);

// 物品实体：继承 Entity。除基础属性外携带价值、攻/防加成、回复量与
// 堆叠信息，use() 按类别分派到玩家（回血/装备），是背包与商店的基本单元。
class Item : public Entity {
public:
    Item(const std::string& name, char symbol, ItemCategory category,
         ItemRarity rarity = ItemRarity::Common);
    ItemCategory getCategory() const { return m_category; }
    ItemRarity getRarity() const { return m_rarity; }
    int getValue() const { return m_value; }
    int getWeight() const { return m_weight; }
    bool isStackable() const { return m_stackable; }
    int getStackSize() const { return m_stackSize; }
    void setStackSize(int size) { m_stackSize = size; }
    int getAttackBonus() const { return m_attackBonus; }
    void setAttackBonus(int bonus) { m_attackBonus = bonus; }
    int getDefenseBonus() const { return m_defenseBonus; }
    void setDefenseBonus(int bonus) { m_defenseBonus = bonus; }
    int getHealAmount() const { return m_healAmount; }
    void setHealAmount(int amount) { m_healAmount = amount; }
    void setValue(int value) { m_value = value; }
    std::string use(Player& player);
    std::string getDescription() const override;
    std::string getFullDescription() const;
    int getDisplayColor() const;
private:
    ItemCategory m_category;
    ItemRarity m_rarity;
    int m_value, m_weight;
    bool m_stackable;
    int m_stackSize;
    int m_attackBonus;
    int m_defenseBonus;
    int m_healAmount;
};
}
