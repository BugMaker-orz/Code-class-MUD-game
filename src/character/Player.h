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
    int getExpToNextLevel() const { return 60 + m_level * 30; }  // 升级曲线：1→2需90，逐级+30（原 等级×100 过慢）
    int getMaxHp() const { return m_maxHp; }
    int getCurrentHp() const { return m_currentHp; }
    int getAttack() const { return m_baseAttack + m_attackBonus; }
    int getDefense() const { return m_baseDefense + m_defenseBonus; }
    int getBaseAttack() const { return m_baseAttack; }    // 基础攻击（不含装备加成）
    int getBaseDefense() const { return m_baseDefense; }  // 基础防御（不含装备加成）
    int getAttackBonus() const { return m_attackBonus; }  // 装备攻击加成
    int getDefenseBonus() const { return m_defenseBonus; } // 装备防御加成
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
    // 读档恢复：覆盖全部成长属性（背包/装备由调用方随后重建）
    void restore(int level, int exp, int maxHp, int hp, int baseAtk, int baseDef, int gold);
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
