#include "character/Player.h"
#include "item/Item.h"
#include <algorithm>
namespace dq {
// 析构：释放背包中所有物品（对象所有权归 Player）
Player::~Player() {
    for (auto it : m_inventory) delete it;
    m_inventory.clear();
}
// 构造函数：初始 1 级、36 血、基础攻 6 防 3；装备加成为 0、初始无装备
Player::Player(const std::string& name, const Position& pos)
    : Entity(name, pos, '@'), m_level(1), m_exp(0), m_maxHp(36), m_currentHp(36),
      m_baseAttack(6), m_baseDefense(3), m_gold(0), m_attackBonus(0), m_defenseBonus(0),
      m_weapon(nullptr), m_armor(nullptr) {}
// 入背包：容量上限 20，超限时静默丢弃
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
// 受伤：生命下限 0，归零即标记死亡
void Player::takeDamage(int damage) {
    m_currentHp -= damage;
    if (m_currentHp < 0) m_currentHp = 0;
    if (m_currentHp == 0) m_isAlive = false;
}
// 治疗：上限为最大生命
void Player::heal(int amount) {
    m_currentHp += amount;
    if (m_currentHp > m_maxHp) m_currentHp = m_maxHp;
}
// 获得经验：用 while 循环消化溢出经验，可连续跨级（每级升级回满血）
void Player::gainExp(int amount) {
    m_exp += amount;
    while (m_exp >= getExpToNextLevel()) {
        m_exp -= getExpToNextLevel();
        levelUp();
    }
}
// 升级：生命上限 +5 并回满，基础攻防各 +1
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
// 装备武器：先卸下旧武器，再挂新武器并把其攻击加成计入 m_attackBonus
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
// 卸下武器：从总加成中扣除该武器加成
void Player::unequipWeapon() {
    if (m_weapon) { m_attackBonus -= m_weapon->getAttackBonus(); m_weapon = nullptr; }
}
void Player::unequipArmor() {
    if (m_armor) { m_defenseBonus -= m_armor->getDefenseBonus(); m_armor = nullptr; }
}
// 读档恢复：覆盖全部成长属性；装备与加成随后由 equip 重建
void Player::restore(int level, int exp, int maxHp, int hp, int baseAtk, int baseDef, int gold) {
    m_level = level;
    m_exp = exp;
    m_maxHp = maxHp;
    m_currentHp = hp;
    m_baseAttack = baseAtk;
    m_baseDefense = baseDef;
    m_gold = gold;
    // 装备加成由随后的 equip 重建，这里先清零
    m_attackBonus = 0;
    m_defenseBonus = 0;
    // 消化溢出经验：读档恢复的高经验可能跨级（升级回满血）
    while (m_exp >= getExpToNextLevel()) {
        m_exp -= getExpToNextLevel();
        levelUp();
    }
    m_weapon = nullptr;
    m_armor = nullptr;
    if (m_currentHp > m_maxHp) m_currentHp = m_maxHp;
    if (m_currentHp <= 0) m_currentHp = 1;
    m_isAlive = true;
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
