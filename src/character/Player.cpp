#include "character/Player.h"
#include "item/Item.h"
#include <algorithm>
namespace dq {
Player::~Player() {
    for (auto it : m_inventory) delete it;
    m_inventory.clear();
}
Player::Player(const std::string& name, const Position& pos)
    : Entity(name, pos, '@'), m_level(1), m_exp(0), m_maxHp(36), m_currentHp(36),
      m_baseAttack(6), m_baseDefense(3), m_gold(0), m_attackBonus(0), m_defenseBonus(0),
      m_weapon(nullptr), m_armor(nullptr) {}
void Player::addItem(Item* item) {
    if (m_inventory.size() < MAX_INVENTORY_SIZE) m_inventory.push_back(item);
}
void Player::removeItem(Item* item) {
    auto it = std::find(m_inventory.begin(), m_inventory.end(), item);
    if (it != m_inventory.end()) m_inventory.erase(it);
}
bool Player::hasItem(const Item* item) const {
    return std::find(m_inventory.begin(), m_inventory.end(), item) != m_inventory.end();
}
void Player::takeDamage(int damage) {
    m_currentHp -= damage;
    if (m_currentHp < 0) m_currentHp = 0;
    if (m_currentHp == 0) m_isAlive = false;
}
void Player::heal(int amount) {
    m_currentHp += amount;
    if (m_currentHp > m_maxHp) m_currentHp = m_maxHp;
}
void Player::gainExp(int amount) {
    m_exp += amount;
    while (m_exp >= getExpToNextLevel()) {
        m_exp -= getExpToNextLevel();
        levelUp();
    }
}
void Player::levelUp() {
    ++m_level;
    m_maxHp += 5;
    m_currentHp = m_maxHp;
    m_baseAttack += 1;
    m_baseDefense += 1;
}
bool Player::spendGold(int amount) {
    if (m_gold >= amount) { m_gold -= amount; return true; }
    return false;
}
void Player::equipWeapon(Item* weapon) {
    if (m_weapon) unequipWeapon();
    m_weapon = weapon;
    if (m_weapon) m_attackBonus += m_weapon->getAttackBonus();
}
void Player::equipArmor(Item* armor) {
    if (m_armor) unequipArmor();
    m_armor = armor;
    if (m_armor) m_defenseBonus += m_armor->getDefenseBonus();
}
void Player::unequipWeapon() {
    if (m_weapon) { m_attackBonus -= m_weapon->getAttackBonus(); m_weapon = nullptr; }
}
void Player::unequipArmor() {
    if (m_armor) { m_defenseBonus -= m_armor->getDefenseBonus(); m_armor = nullptr; }
}
std::string Player::getDescription() const {
    std::string desc = m_name + " 等级" + std::to_string(m_level)
        + " 生命 " + std::to_string(m_currentHp) + "/" + std::to_string(m_maxHp)
        + " 攻击 " + std::to_string(getAttack()) + " 防御 " + std::to_string(getDefense());
    if (m_weapon) desc += " 武器:" + m_weapon->getName();
    if (m_armor) desc += " 护甲:" + m_armor->getName();
    return desc;
}
}
