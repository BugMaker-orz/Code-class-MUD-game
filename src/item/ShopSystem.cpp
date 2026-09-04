#include "item/ShopSystem.h"
#include "item/DropSystem.h"
#include "character/Player.h"
#include <cstdlib>
namespace dq {
ShopSystem::~ShopSystem() {
    for (auto& si : m_items) delete si.item;
    m_items.clear();
}
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
