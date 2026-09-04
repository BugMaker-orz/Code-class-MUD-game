#pragma once
#include "map/Map.h"
#include "character/Player.h"
#include "character/Monster.h"
#include "character/Npc.h"
#include "item/Item.h"
#include "item/ShopSystem.h"
#include <vector>
#include <string>
namespace dq {
class GameContext {
public:
    enum class GameState { Menu, Playing, Inventory, Shop, Talk, Dead, Victory };
    Map* currentMap = nullptr;
    Player* player = nullptr;
    std::vector<Monster*> monsters;
    std::vector<Npc*> npcs;
    std::vector<Item*> itemsOnGround;
    std::vector<std::string> messageLog;
    std::vector<std::string> pendingDialogue;
    ShopSystem* currentShop = nullptr;
    GameState state = GameState::Menu;
    int currentLevel = 1;
    bool isGameRunning = false;
    void addMessage(const std::string& msg) { messageLog.push_back(msg); }
    void clearMessages() { messageLog.clear(); }
    void addMonster(Monster* monster) { monsters.push_back(monster); }
    void removeMonster(Monster* monster);
    void addNpc(Npc* npc) { npcs.push_back(npc); }
    void removeNpc(Npc* npc);
    void addItemOnGround(Item* item) { itemsOnGround.push_back(item); }
    void removeItemOnGround(Item* item);
    void cleanup();
};
}
