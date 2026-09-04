#include "item/ShopSystem.h"
#include "item/DropSystem.h"
#include "character/Player.h"
#include <cstdlib>
namespace dq {
// 析构：释放货架上所有商品对象
ShopSystem::~ShopSystem() {
    for (auto& si : m_items) delete si.item;
    m_items.clear();
}
// 生成货架：按店铺类型分派对应商品池，生成前先清空旧货架
void ShopSystem::generateShopItems(int level, int count) {
    // 清理旧物品
    for (auto& si : m_items) delete si.item;
    m_items.clear();
    switch (m_shopType) {
        case ShopType::General: generateGeneralShop(level, count); break;
        case ShopType::Weapon: generateWeaponShop(level, count); break;
        case ShopType::Armor: generateArmorShop(level, count); break;
        default: generateGeneralShop(level, count);
    }
}
void ShopSystem::generateShopItems(const std::vector<ShopItem>& items) {
    for (auto& si : m_items) delete si.item;
    m_items = items;  // 注意：这里会复制指针，所有权转移给 ShopSystem
}
// 购买：校验下标/库存/金币，成功则复制商品入背包并扣款
std::string ShopSystem::buyItem(Player& player, int index, int quantity) {
    if (index < 0 || index >= static_cast<int>(m_items.size()))
        return "没有这个商品。";
    ShopItem& si = m_items[index];
    int totalCost = si.buyPrice * quantity;
    if (quantity > 1 && si.quantity > 0 && si.quantity < quantity)
        return "库存不足。";
    if (!player.spendGold(totalCost))
        return "金币不足，买不起啊！";
    for (int i = 0; i < quantity; ++i) {
        Item* newItem = new Item(*si.item);
        player.addItem(newItem);
    }
    if (si.quantity > 0) si.quantity -= quantity;
    return "你购买了 " + si.item->getName() + " x" + std::to_string(quantity)
        + "，花费 " + std::to_string(totalCost) + " 金币。";
}
// 出售：按回收价结算金币，并从背包移除/释放该物品
std::string ShopSystem::sellItem(Player& player, int inventoryIndex) {
    auto& inv = player.getInventory();
    if (inventoryIndex < 0 || inventoryIndex >= static_cast<int>(inv.size()))
        return "背包里没有这个物品。";
    Item* item = inv[inventoryIndex];
    int sellPrice = calculateSellPrice(*item);
    player.addGold(sellPrice);
    player.removeItem(item);
    delete item;
    return "你卖出了物品，获得 " + std::to_string(sellPrice) + " 金币。";
}
std::vector<std::string> ShopSystem::getBuyableDescriptions() const {
    std::vector<std::string> desc;
    for (const auto& si : m_items) {
        desc.push_back(si.item->getName() + " - " + std::to_string(si.buyPrice) + " gold");
    }
    return desc;
}
// 价格规则：买入 = 价值 ×2，卖出 = 价值 /2（商店吃一半差价）
int ShopSystem::calculateBuyPrice(const Item& item) {
    return item.getValue() * 2;
}
int ShopSystem::calculateSellPrice(const Item& item) {
    return item.getValue() / 2;
}
void ShopSystem::generateGeneralShop(int level, int count) {
    for (int i = 0; i < count; ++i) {
        Item* item = DropSystem::generateRandomItem(level);
        ShopItem si;
        si.item = item;
        si.buyPrice = calculateBuyPrice(*item);
        si.sellPrice = calculateSellPrice(*item);
        si.quantity = -1;
        m_items.push_back(si);
    }
}
void ShopSystem::generateWeaponShop(int level, int count) {
    for (int i = 0; i < count; ++i) {
        Item* item = DropSystem::generateRandomWeapon(level);
        ShopItem si;
        si.item = item;
        si.buyPrice = calculateBuyPrice(*item);
        si.sellPrice = calculateSellPrice(*item);
        si.quantity = -1;
        m_items.push_back(si);
    }
}
void ShopSystem::generateArmorShop(int level, int count) {
    for (int i = 0; i < count; ++i) {
        Item* item = DropSystem::generateRandomArmor(level);
        ShopItem si;
        si.item = item;
        si.buyPrice = calculateBuyPrice(*item);
        si.sellPrice = calculateSellPrice(*item);
        si.quantity = -1;
        m_items.push_back(si);
    }
}
}
