#pragma once
#include "core/Entity.h"
#include <string>
namespace dq {
class Player;
enum class ItemCategory { Weapon, Armor, Potion, Scroll, Gold, Key, Food, QuestItem };
enum class ItemRarity { Common, Uncommon, Rare, Epic, Legendary };

std::string categoryToChinese(ItemCategory category);
std::string rarityToChinese(ItemRarity rarity);

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
