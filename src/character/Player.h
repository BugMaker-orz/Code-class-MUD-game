#pragma once
#include "core/Entity.h"
#include <vector>
namespace dq {
class Item;
class Player : public Entity {
public:
    Player(const std::string& name, const Position& pos);
    ~Player() override;  // 释放背包中的所有物品
    int getLevel() const { return m_level; }
    int getExp() const { return m_exp; }
    int getExpToNextLevel() const { return m_level * 100; }
    int getMaxHp() const { return m_maxHp; }
    int getCurrentHp() const { return m_currentHp; }
    int getAttack() const { return m_baseAttack + m_attackBonus; }
    int getDefense() const { return m_baseDefense + m_defenseBonus; }
    int getGold() const { return m_gold; }
    const std::vector<Item*>& getInventory() const { return m_inventory; }
    std::vector<Item*>& getInventory() { return m_inventory; }
    void addItem(Item* item);
    void removeItem(Item* item);
    bool hasItem(const Item* item) const;
    int getInventorySize() const { return m_inventory.size(); }
    void takeDamage(int damage);
    void heal(int amount);
    void gainExp(int amount);
    void levelUp();
    void addGold(int amount) { m_gold += amount; }
    bool spendGold(int amount);
    Item* getWeapon() const { return m_weapon; }
    Item* getArmor() const { return m_armor; }
    void equipWeapon(Item* weapon);
    void equipArmor(Item* armor);
    void unequipWeapon();
    void unequipArmor();
    std::string getDescription() const override;
private:
    int m_level, m_exp;
    int m_maxHp, m_currentHp;
    int m_baseAttack, m_baseDefense;
    int m_gold;
    int m_attackBonus, m_defenseBonus;
    Item* m_weapon;
    Item* m_armor;
    std::vector<Item*> m_inventory;
    static constexpr int MAX_INVENTORY_SIZE = 20;
};
}
