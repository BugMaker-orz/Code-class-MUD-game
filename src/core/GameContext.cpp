#include "core/GameContext.h"
#include <algorithm>
namespace dq {
// 从场景移除实体：只移除不释放，对象所有权统一由 cleanup() 管理
void GameContext::removeMonster(Monster* monster) {
    auto it = std::find(monsters.begin(), monsters.end(), monster);
    if (it != monsters.end()) monsters.erase(it);
}
void GameContext::removeNpc(Npc* npc) {
    auto it = std::find(npcs.begin(), npcs.end(), npc);
    if (it != npcs.end()) npcs.erase(it);
}
void GameContext::removeItemOnGround(Item* item) {
    auto it = std::find(itemsOnGround.begin(), itemsOnGround.end(), item);
    if (it != itemsOnGround.end()) itemsOnGround.erase(it);
}
// 清理：统一释放本帧/本局全部动态对象（怪物、地面物品、NPC、地图、玩家、商店）
void GameContext::cleanup() {
    for (auto m : monsters) delete m;
    for (auto i : itemsOnGround) delete i;
    for (auto n : npcs) delete n;
    monsters.clear();
    itemsOnGround.clear();
    npcs.clear();
    delete currentMap;
    currentMap = nullptr;
    delete player;
    player = nullptr;
    delete currentShop;
    currentShop = nullptr;
}
}
