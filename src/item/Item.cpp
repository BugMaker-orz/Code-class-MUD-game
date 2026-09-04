#include "item/Item.h"
#include "character/Player.h"
namespace dq {
std::string categoryToChinese(ItemCategory category) {
    switch (category) {
        case ItemCategory::Weapon: return "武器";
        case ItemCategory::Armor: return "护甲";
        case ItemCategory::Potion: return "药水";
        case ItemCategory::Scroll: return "卷轴";
        case ItemCategory::Gold: return "金币";
        case ItemCategory::Key: return "钥匙";
        case ItemCategory::Food: return "食物";
        case ItemCategory::QuestItem: return "任务物品";
        default: return "未知";
    }
}
std::string rarityToChinese(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::Common: return "普通";
        case ItemRarity::Uncommon: return "精良";
        case ItemRarity::Rare: return "稀有";
        case ItemRarity::Epic: return "史诗";
        case ItemRarity::Legendary: return "传说";
        default: return "未知";
    }
}
// 构造函数：默认价值为 1，按类别覆盖（药水 10 / 武器、护甲 20 / 金币 1 / 其他 5）
Item::Item(const std::string& name, char symbol, ItemCategory category, ItemRarity rarity)
    : Entity(name, Position(0,0), symbol), m_category(category), m_rarity(rarity),
      m_value(1), m_weight(1), m_stackable(false), m_stackSize(1),
      m_attackBonus(0), m_defenseBonus(0), m_healAmount(0) {
    switch (category) {
        case ItemCategory::Potion: m_value = 10; break;
        case ItemCategory::Weapon: m_value = 20; break;
        case ItemCategory::Armor: m_value = 20; break;
        case ItemCategory::Gold: m_value = 1; break;
        default: m_value = 5;
    }
}
// 使用物品：按类别分派——药水/食物回血、武器/护甲直接装备
std::string Item::use(Player& player) {
    switch (m_category) {
        case ItemCategory::Potion:
            player.heal(m_healAmount);
            return "你喝下" + m_name + "，恢复了 " + std::to_string(m_healAmount) + " 点生命。";
        case ItemCategory::Weapon:
            player.equipWeapon(this);
            return "你装备了" + m_name + "，攻击提升 " + std::to_string(m_attackBonus) + "。";
        case ItemCategory::Armor:
            player.equipArmor(this);
            return "你装备了" + m_name + "，防御提升 " + std::to_string(m_defenseBonus) + "。";
        case ItemCategory::Food:
            player.heal(m_healAmount > 0 ? m_healAmount : 5);
            return "你吃下了" + m_name + "，恢复了体力。";
        default:
            return "这个东西现在用不了。";
    }
}
std::string Item::getDescription() const { return m_name; }
// 完整描述：稀有度 + 名称 + 类别，附攻/防/恢复数值与价值
std::string Item::getFullDescription() const {
    std::string desc = "[" + rarityToChinese(m_rarity) + "]" + m_name + "（" + categoryToChinese(m_category);
    if (m_attackBonus > 0) desc += "，攻击+" + std::to_string(m_attackBonus);
    if (m_defenseBonus > 0) desc += "，防御+" + std::to_string(m_defenseBonus);
    if (m_healAmount > 0) desc += "，恢复" + std::to_string(m_healAmount);
    desc += "）价值 " + std::to_string(m_value) + " 金币";
    return desc;
}
int Item::getDisplayColor() const { return 37; }
}
