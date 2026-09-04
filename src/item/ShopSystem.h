#pragma once
#include "item/Item.h"
#include <vector>
#include <string>
namespace dq {
class Player;
struct ShopItem {
    Item* item;
    int buyPrice;
    int sellPrice;
    int quantity;
};
class ShopSystem {
public:
    enum class ShopType { General, Weapon, Armor, BlackMarket };
    ShopSystem() = default;
    ~ShopSystem();  // 释放所有物品
    void generateShopItems(int level, int count = 6);
    void generateShopItems(const std::vector<ShopItem>& items);
    const std::vector<ShopItem>& getItems() const { return m_items; }
    // 返回中文反馈消息：空串表示成功
    std::string buyItem(Player& player, int index, int quantity = 1);
    std::string sellItem(Player& player, int inventoryIndex);
    std::vector<std::string> getBuyableDescriptions() const;
    void setShopType(ShopType type) { m_shopType = type; }
    ShopType getShopType() const { return m_shopType; }
    static int calculateBuyPrice(const Item& item);
    static int calculateSellPrice(const Item& item);
private:
    std::vector<ShopItem> m_items;
    ShopType m_shopType = ShopType::General;
    void generateGeneralShop(int level, int count);
    void generateWeaponShop(int level, int count);
    void generateArmorShop(int level, int count);
};
}
