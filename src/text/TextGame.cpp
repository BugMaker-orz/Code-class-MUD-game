#include "text/TextGame.h"
#include "map/DungeonGenerator.h"
#include "combat/CombatSystem.h"
#include "item/DropSystem.h"
#include <iostream>
#include <sstream>
#include <optional>
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace dq {
// ==================== ANSI 彩色文本 ====================
// 正文一律白色（终端默认/显式白），标题用彩色区分层次，危险提示用鲜红。
// 若终端不支持 ANSI（极老终端），颜色码会原样显示，可改用 Windows Terminal。
const std::string C_RED     = "\033[91m";    // 鲜红（危险/敌人）
const std::string C_GREEN   = "\033[92m";    // 绿（信息标题）
const std::string C_YELLOW  = "\033[93m";    // 黄（房间名/NPC/奖励）
const std::string C_BLUE    = "\033[94m";    // 亮蓝（小地图当前房间）
const std::string C_MAGENTA = "\033[95m";    // 品红（状态标题/魔王）
const std::string C_CYAN    = "\033[96m";    // 青（主标题/小地图标题）
const std::string C_ORANGE  = "\033[38;5;208m"; // 橙（强力资源）
const std::string C_WHITE   = "\033[37m";    // 白（正文）
const std::string C_RESET   = "\033[0m";

std::string TextGame::col(const std::string& code, const std::string& text) {
    return code + text + C_RESET;
}

TextGame::TextGame() {
    m_ctx.currentShop = new ShopSystem();
}
TextGame::~TextGame() { cleanup(); }

void TextGame::cleanup() {
    for (auto m : m_ctx.monsters) delete m;
    for (auto n : m_ctx.npcs) delete n;
    for (auto i : m_ctx.itemsOnGround) delete i;
    m_ctx.monsters.clear(); m_ctx.npcs.clear(); m_ctx.itemsOnGround.clear();
    delete m_ctx.currentMap; m_ctx.currentMap = nullptr;
    delete m_ctx.player; m_ctx.player = nullptr;
    delete m_ctx.currentShop; m_ctx.currentShop = nullptr;
}

void TextGame::clearLevelEntities() {
    for (auto m : m_ctx.monsters) delete m;
    for (auto n : m_ctx.npcs) delete n;
    for (auto i : m_ctx.itemsOnGround) delete i;
    m_ctx.monsters.clear(); m_ctx.npcs.clear(); m_ctx.itemsOnGround.clear();
    m_guideHp.clear();                       // 旧层向导的血量记录一并失效
    delete m_ctx.currentMap; m_ctx.currentMap = nullptr;
}

static std::optional<Position> findFreeTileIn(Map& map, const Map::Room& room,
                                       std::vector<Position>& used,
                                       const Position& avoid = Position(-1,-1)) {
    for (int y = room.topLeft.y; y <= room.bottomRight.y; ++y)
        for (int x = room.topLeft.x; x <= room.bottomRight.x; ++x) {
            Position pos(x, y);
            if (!map.isWalkable(pos)) continue;
            TileType t = map.getTile(pos);
            if (t == TileType::StairsDown || t == TileType::StairsUp || t == TileType::Door) continue;
            if (pos == avoid) continue;
            bool taken = false;
            for (auto& u : used) if (u == pos) { taken = true; break; }
            if (taken) continue;
            used.push_back(pos);
            return pos;
        }
    return std::nullopt;
}
static MonsterType randomMonsterType(int lvl) {
    switch (rand() % 5) {
        case 0: return MonsterType::Goblin;
        case 1: return MonsterType::Skeleton;
        case 2: return MonsterType::Bat;
        case 3: return MonsterType::Slime;
        default: return MonsterType::Orc;
    }
}

void TextGame::newGame() {
    cleanup();
    m_ctx.currentLevel = 1;
    m_betrayedGuide = false;
    m_guideHp.clear();
    m_ctx.player = new Player("冒险者", Position(0, 0));
    m_ctx.currentShop = new ShopSystem();
    m_ctx.state = GameContext::GameState::Playing;
    m_log.clear();
    generateLevel();
}

void TextGame::generateLevel() {
    clearLevelEntities();
    DungeonGenerator gen;
    Map* map = new Map(gen.generateDungeon(m_ctx.currentLevel, 5));
    m_ctx.currentMap = map;
    const auto& rooms = map->getRooms();
    if (rooms.empty()) return;
    int lvl = m_ctx.currentLevel;

    // 全视野
    for (int x = 0; x < map->getWidth(); ++x)
        for (int y = 0; y < map->getHeight(); ++y) { map->setVisible(x, y, true); map->setExplored(x, y, true); }

    std::vector<Position> used;
    for (size_t i = 0; i < rooms.size(); ++i) {
        const auto& room = rooms[i];
        switch (room.type) {
            case Map::RoomType::Start: {
                m_currentRoom = static_cast<int>(i);
                auto pos = findFreeTileIn(*map, room, used, room.getCenter());
                if (pos) {
                    // 每一层的向导不同：人设、台词、剧情都随层数推进
                    Npc* guide = new Npc(Npc::guideName(lvl), '?', *pos, Npc::NpcType::Guide);
                    guide->setDialogue(Npc::guideDialogue(lvl));
                    m_ctx.addNpc(guide);
                }
                break;
            }
            case Map::RoomType::Shop: {
                auto pos = findFreeTileIn(*map, room, used);
                if (pos) {
                    Npc* merchant = new Npc("商人", '$', *pos, Npc::NpcType::Merchant);
                    merchant->setDialogue(Npc::merchantDialogue(lvl));
                    m_ctx.addNpc(merchant);
                }
                break;
            }
            case Map::RoomType::Treasure: {
                // 宝箱房：地上放一个宝箱（金币 + 一件装备/物品）
                auto pos = findFreeTileIn(*map, room, used);
                if (pos) {
                    Item* gold = new Item("金币", '$', ItemCategory::Gold);
                    gold->setValue(15 + lvl * 5);
                    gold->setPosition(*pos);
                    m_ctx.addItemOnGround(gold);
                }
                auto pos2 = findFreeTileIn(*map, room, used);
                if (pos2) {
                    Item* it = DropSystem::generateRandomItem(lvl);
                    it->setPosition(*pos2);
                    m_ctx.addItemOnGround(it);
                }
                break;
            }
            case Map::RoomType::Boss: {
                auto pos = findFreeTileIn(*map, room, used);
                m_ctx.addMonster(new Monster(MonsterType::Boss, pos ? *pos : room.getCenter(), lvl));
                break;
            }
            case Map::RoomType::StairsDown: {
                Position c = room.getCenter();
                map->setTile(c, TileType::StairsDown);
                auto pos = findFreeTileIn(*map, room, used, c);
                if (pos) m_ctx.addMonster(new Monster(MonsterType::Goblin, *pos, lvl));
                break;
            }
            default: {
                int n = 1 + rand() % 2;
                for (int k = 0; k < n; ++k) {
                    auto pos = findFreeTileIn(*map, room, used);
                    if (!pos) break;
                    m_ctx.addMonster(new Monster(randomMonsterType(lvl), *pos, lvl));
                }
                break;
            }
        }
    }
    m_ctx.player->setPosition(rooms[m_currentRoom].getCenter());
    m_prevRoom = m_currentRoom;
    appendLog("你来到了地牢第 " + std::to_string(lvl) + " 层（共 5 层）。");
    appendLog("你身处【" + roomTypeName(rooms[m_currentRoom]) + "】。");
}

void TextGame::nextLevel() {
    m_ctx.currentLevel++;
    generateLevel();
    m_ctx.player->heal(6);
    appendLog("你沿着楼梯来到更深的第 " + std::to_string(m_ctx.currentLevel) + " 层！恢复 6 点生命。");
}

std::string TextGame::roomTypeName(const Map::Room& r) const {
    switch (r.type) {
        case Map::RoomType::Start: return "起点房";
        case Map::RoomType::Normal: return "战斗房";
        case Map::RoomType::Treasure: return "宝箱房";
        case Map::RoomType::Shop: return "商店房";
        case Map::RoomType::StairsDown: return "楼梯房";
        case Map::RoomType::Boss: return "魔王房";
        default: return "走廊";
    }
}
std::string TextGame::monsterChineseName(MonsterType t) const {
    switch (t) {
        case MonsterType::Goblin: return "哥布林";
        case MonsterType::Skeleton: return "骷髅兵";
        case MonsterType::Bat: return "蝙蝠";
        case MonsterType::Slime: return "史莱姆";
        case MonsterType::Orc: return "兽人";
        case MonsterType::Boss: return "魔王";
        default: return "怪物";
    }
}

const Map::Room& TextGame::curRoom() {
    const auto& rooms = m_ctx.currentMap->getRooms();
    return rooms[m_currentRoom];
}

std::string TextGame::trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
std::string TextGame::toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}
// 取命令 token 之后的参数（按 token 实际字节数裁剪，安全处理中文）
static std::string argAfter(const std::string& line, const std::string& token) {
    size_t p = line.find(token);
    if (p == std::string::npos) return "";
    std::string r = line.substr(p + token.size());
    size_t a = r.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = r.find_last_not_of(" \t\r\n");
    return r.substr(a, b - a + 1);
}

void TextGame::appendLog(const std::string& msg) {
    m_log.push_back(msg);
    if (m_log.size() > 200) m_log.erase(m_log.begin());   // 保留足够历史，避免旧信息过早消失
}

// ==================== 渲染 ====================
void TextGame::renderMiniMap(std::ostream& out) {
    // 房间拓扑小地图（纯文字）：节点 [X]，横向 - 纵向 |
    const auto& rooms = m_ctx.currentMap->getRooms();
    int cols = 3, rows = 2;
    for (const auto& r : rooms) {
        if (r.gridX + 1 > cols) cols = r.gridX + 1;
        if (r.gridY + 1 > rows) rows = r.gridY + 1;
    }
    // 按 grid 填字符
    std::vector<std::string> g(rows * 2 - 1, std::string(cols * 4 - 1, ' '));
    for (const auto& r : rooms) {
        int x = r.gridX * 4, y = r.gridY * 2;
        char c;
        if (m_currentRoom == static_cast<int>(&r - &rooms[0])) c = '@';
        else {
            switch (r.type) {
                case Map::RoomType::Start: c = 'S'; break;
                case Map::RoomType::Normal: c = 'F'; break;
                case Map::RoomType::Treasure: c = 'T'; break;
                case Map::RoomType::Shop: c = '$'; break;
                case Map::RoomType::StairsDown: c = '>'; break;
                case Map::RoomType::Boss: c = 'B'; break;
                default: c = '?'; break;
            }
        }
        g[y][x] = '['; g[y][x + 1] = c; g[y][x + 2] = ']';
        // 横向连线
        if (r.gridX < cols - 1) {
            int nx = (r.gridX + 1) * 4;
            bool connected = false;
            for (int oi : r.connectedRooms) {
                const auto& oroom = rooms[oi];
                if (oroom.gridY == r.gridY && oroom.gridX == r.gridX + 1) connected = true;
            }
            if (connected) g[y][x + 3] = '-';
        }
        // 纵向连线
        if (r.gridY < rows - 1) {
            bool connected = false;
            for (int oi : r.connectedRooms) {
                const auto& oroom = rooms[oi];
                if (oroom.gridX == r.gridX && oroom.gridY == r.gridY + 1) connected = true;
            }
            if (connected && y + 1 < static_cast<int>(g.size())) g[y + 1][x + 1] = '|';
        }
    }
    // 逐行输出：当前房间节点亮蓝，魔王节点鲜红，其余白色
    int curMX = -1, curMY = -1;
    for (size_t i = 0; i < rooms.size(); ++i)
        if (static_cast<int>(i) == m_currentRoom) { curMX = rooms[i].gridX * 4; curMY = rooms[i].gridY * 2; }
    for (int y = 0; y < static_cast<int>(g.size()); ++y) {
        out << "    ";
        for (int x = 0; x < static_cast<int>(g[y].size()); ++x) {
            char ch = g[y][x];
            if (y == curMY && x >= curMX && x <= curMX + 2)
                out << col(C_BLUE, std::string(1, ch));
            else if (ch == 'B')
                out << col(C_RED, std::string(1, ch));
            else
                out << ch;
        }
        out << "\n";
    }
}

void TextGame::renderRoomDesc(std::ostream& out) {
    const auto& rooms = m_ctx.currentMap->getRooms();
    const Map::Room& room = curRoom();
    out << col(C_YELLOW, "你来到了【" + roomTypeName(room) + "】。") << "\n";

    // 房间里的怪物
    std::vector<Monster*> hereM;
    for (Monster* m : m_ctx.monsters)
        if (m->isAlive() && room.contains(m->getPosition())) hereM.push_back(m);
    if (!hereM.empty()) {
        out << "房间里有怪物：\n";
        for (Monster* m : hereM) {
            out << "  · " << col(C_RED, monsterChineseName(m->getMonsterType())) << "（HP "
                << m->getCurrentHp() << "/" << m->getMaxHp() << "，攻 " << m->getAttack()
                << "，防 " << m->getDefense() << "）\n";
        }
        out << "  可输入「攻击 名字」战斗，或「逃跑」离开。\n";
    } else {
        out << "房间里没有怪物，很安全。\n";
    }

    // 地面物品（宝箱内容）
    std::vector<Item*> hereI;
    for (Item* it : m_ctx.itemsOnGround)
        if (room.contains(it->getPosition())) hereI.push_back(it);
    if (!hereI.empty()) {
        out << "地上有东西：\n";
        for (Item* it : hereI) {
            std::string c;
            if (it->getCategory() == ItemCategory::Gold) c = "金币 ×" + std::to_string(it->getValue());
            else if (it->getCategory() == ItemCategory::Potion) c = "药水";
            else if (it->getCategory() == ItemCategory::Weapon) c = "武器";
            else if (it->getCategory() == ItemCategory::Armor) c = "护甲";
            else c = it->getName();
            out << "  · " << c << "\n";
        }
        if (room.type == Map::RoomType::Treasure)
            out << "  这是宝箱房！输入「开宝箱」打开它。\n";
        else
            out << "  输入「拾取」捡起来。\n";
    }

    // NPC
    bool hasNpc = false;
    for (Npc* n : m_ctx.npcs)
        if (room.contains(n->getPosition())) {
            hasNpc = true;
            out << "这里有 NPC：" << col(C_YELLOW, n->getName()) << "（" << (n->isMerchant() ? "商人" : "向导") << "）。\n";
            if (n->isMerchant()) out << "  输入「交易」买卖装备。\n";
            else {
                out << "  输入「对话」听听故事。\n";
                // 鲜红提示：任何时候遇到向导都可以攻击（高风险高回报，且改变结局）
                out << col(C_RED, "  ⚠ 你可以攻击向导！输入「攻击 向导」夺取强力资源") << "\n"
                    << col(C_RED, "    （背叛者将无法以勇者之名通关——击败魔王后，你会成为新的魔王）") << "\n";
            }
        }
    (void)hasNpc;

    // 楼梯
    bool hasStairs = false;
    for (int x = room.topLeft.x; x <= room.bottomRight.x; ++x)
        for (int y = room.topLeft.y; y <= room.bottomRight.y; ++y)
            if (m_ctx.currentMap->getTile(x, y) == TileType::StairsDown) hasStairs = true;
    if (hasStairs) {
        if (m_ctx.currentLevel >= 5)
            out << "这里有一道通往深渊的门扉，但已被封死（第 5 层是终点）。\n";
        else
            out << "这里有向下的楼梯『>』！输入「下楼」前往下一层。\n";
    }

    // 出口描述
    const auto& cr = room;
    std::vector<std::string> exits;
    for (int oi : cr.connectedRooms) {
        const auto& nr = rooms[oi];
        std::string dir;
        if (nr.gridX == cr.gridX + 1) dir = "东";
        else if (nr.gridX == cr.gridX - 1) dir = "西";
        else if (nr.gridY == cr.gridY + 1) dir = "南";
        else if (nr.gridY == cr.gridY - 1) dir = "北";
        if (!dir.empty()) exits.push_back(dir + "→" + roomTypeName(nr));
    }
    if (!exits.empty()) {
        out << "出口：";
        for (size_t i = 0; i < exits.size(); ++i) {
            out << exits[i];
            if (i + 1 < exits.size()) out << "，";
        }
        out << "（输入「去 方向」移动）\n";
    }
}

void TextGame::renderStatus(std::ostream& out) {
    Player* p = m_ctx.player;
    out << col(C_MAGENTA, "【状态】") << col(C_BLUE, "【勇者】")
        << " " << col(C_RED, "HP " + std::to_string(p->getCurrentHp()) + "/" + std::to_string(p->getMaxHp()))
        << " | " << col(C_YELLOW, "等级 " + std::to_string(p->getLevel()))
        << " | " << "攻 " << p->getAttack()
        << " | " << "防 " << p->getDefense()
        << " | " << col(C_ORANGE, "金币 " + std::to_string(p->getGold()))
        << " | " << col(C_GREEN, "经验 " + std::to_string(p->getExp()) + "/" + std::to_string(p->getExpToNextLevel()))
        << "\n";
}

void TextGame::renderLog(std::ostream& out) {
    if (m_log.empty()) return;
    out << col(C_GREEN, "【信息】") << "\n";
    // 每屏只显示最近 15 条，更早的信息用「历史」命令查看
    size_t start = (m_log.size() > 15) ? m_log.size() - 15 : 0;
    for (size_t i = start; i < m_log.size(); ++i)
        out << "· " << m_log[i] << "\n";
    if (m_log.size() > 15)
        out << "… 更早的信息输入「历史」查看\n";
}

void TextGame::renderTrade(std::ostream& out) {
    ShopSystem* shop = m_ctx.currentShop;
    out << col(C_CYAN, "===== 商店 =====") << "\n";
    int i = 0;
    for (const auto& si : shop->getItems()) {
        Item* it = si.item;
        out << "  [" << i + 1 << "] " << it->getFullDescription()
            << " | 买 " << si.buyPrice << " 金币\n";
        ++i;
    }
    Player* p = m_ctx.player;
    out << "  你的金币：" << p->getGold() << "\n";
    out << "  输入「买 N」购买 | 「卖 背包序号」出售 | 「离开」返回\n";
}

void TextGame::renderFrame() {
    std::cout << "\n";
    std::cout << col(C_CYAN, "==============================================") << "\n";
    std::cout << col(C_CYAN, "  地牢探险（文字版）· 第 " + std::to_string(m_ctx.currentLevel) + " 层 / 共 5 层") << "\n";
    std::cout << col(C_CYAN, "==============================================") << "\n";
    std::cout << col(C_CYAN, "【小地图】当前层房间布局：") << "\n";
    renderMiniMap(std::cout);
    std::cout << "  节点：[@]你 [S]起点 [F]战斗 [T]宝箱 [$]商店 [>]楼梯 [B]魔王\n\n";
    std::cout << col(C_CYAN, "【当前位置】") << "\n";
    renderRoomDesc(std::cout);
    std::cout << "\n";
    renderLog(std::cout);
    std::cout << "\n";
    renderStatus(std::cout);
    std::cout << "\n";
    std::cout.flush();   // 交互模式：立即输出，避免管道/终端缓冲
}

// ==================== 命令 ====================
void TextGame::handleCommand(const std::string& line) {
    std::string cmd = trim(line);
    if (cmd.empty()) return;
    std::string low = toLower(cmd);

    // 交易子界面命令
    if (m_trading) {
        if (low == "离开" || low == "q" || low == "quit" || low == "close") {
            m_trading = false;
            appendLog("你离开了商店。");
            return;
        }
        if (low.rfind("买", 0) == 0 || low.rfind("buy", 0) == 0) {
            std::string rest = (low.rfind("买", 0) == 0) ? argAfter(cmd, "买") : argAfter(cmd, "buy");
            if (rest.empty()) { appendLog("用法：买 编号"); return; }
            int idx = atoi(rest.c_str());
            std::string r = m_ctx.currentShop->buyItem(*m_ctx.player, idx - 1);
            appendLog(r.empty() ? "购买成功！" : r);
            return;
        }
        if (low.rfind("卖", 0) == 0 || low.rfind("sell", 0) == 0) {
            std::string rest = (low.rfind("卖", 0) == 0) ? argAfter(cmd, "卖") : argAfter(cmd, "sell");
            if (rest.empty()) { appendLog("用法：卖 背包序号"); return; }
            int idx = atoi(rest.c_str());
            std::string r = m_ctx.currentShop->sellItem(*m_ctx.player, idx - 1);
            appendLog(r.empty() ? "出售成功！" : r);
            return;
        }
        appendLog("商店里只能输入：买 N / 卖 N / 离开。");
        return;
    }

    // 方向 / 移动
    if (low == "东" || low == "西" || low == "南" || low == "北" ||
        low.rfind("去", 0) == 0 || low.rfind("到", 0) == 0 || low.rfind("走", 0) == 0 ||
        low.rfind("go", 0) == 0 || low.rfind("move", 0) == 0) {
        std::string dir;
        if (low == "东" || low == "西" || low == "南" || low == "北") dir = low;
        else {
            // 去掉 去/到/走/go/move 前缀
            for (auto& pfx : {"去", "到", "走", "go", "move"}) {
                if (low.rfind(pfx, 0) == 0) { dir = trim(cmd.substr(std::string(pfx).size())); break; }
            }
        }
        if (dir.empty()) { appendLog("用法：去 东/西/南/北"); return; }
        tryMove(dir);
        return;
    }
    // 攻击
    if (low == "攻击" || low == "打" || low == "attack" || low == "atk" ||
        low.rfind("攻击", 0) == 0 || low.rfind("打 ", 0) == 0 || low.rfind("attack ", 0) == 0 ||
        low.rfind("atk ", 0) == 0) {
        std::string target;
        if (low == "攻击" || low == "打" || low == "attack" || low == "atk") target = "";
        else {
            if (low.rfind("攻击", 0) == 0) target = argAfter(cmd, "攻击");
            else if (low.rfind("打 ", 0) == 0) target = argAfter(cmd, "打 ");
            else if (low.rfind("attack ", 0) == 0) target = argAfter(cmd, "attack ");
            else target = argAfter(cmd, "atk ");
        }
        tryAttack(target);
        return;
    }
    // 逃跑
    if (low == "逃跑" || low == "逃" || low == "flee" || low == "run") { tryFlee(); return; }
    // 开宝箱
    if (low == "开宝箱" || low == "宝箱" || low == "打开" || low == "open") { tryOpenTreasure(); return; }
    // 对话
    if (low == "对话" || low == "聊天" || low == "talk" || low == "说") { tryTalk(); return; }
    // 交易
    if (low == "交易" || low == "商店" || low == "买卖" || low == "shop" || low == "trade") { tryTrade(); return; }
    // 拾取
    if (low == "拾取" || low == "捡" || low == "捡起" || low == "pick" || low == "get") { tryPickup(); return; }
    // 背包
    if (low == "背包" || low == "物品" || low == "inv" || low == "bag" || low == "i") { showInventory(); return; }
    // 使用
    if (low.rfind("使用", 0) == 0 || low.rfind("use", 0) == 0) {
        std::string rest = (low.rfind("使用", 0) == 0) ? argAfter(cmd, "使用") : argAfter(cmd, "use");
        tryUseItem(rest);
        return;
    }
    // 下楼
    if (low == "下楼" || low == "下去" || low == "down" || low == "stairs") { tryDownStairs(); return; }
    // 等待
    if (low == "等待" || low == "wait" || low == "空格") { doWait(); return; }
    // 查看
    if (low == "查看" || low == "look" || low == "看" || low == "re") { return; }  // 仅重渲染
    // 历史信息（查看完整日志，不随新信息消失）
    if (low == "历史" || low == "history" || low == "log" || low == "日志") { showHistory(); return; }
    // 帮助
    if (low == "帮助" || low == "help" || low == "?") { printHelp(); return; }
    // 退出
    if (low == "退出" || low == "quit" || low == "q" || low == "exit") { m_running = false; return; }

    appendLog("无法理解「" + cmd + "」。输入 help 查看帮助。");
}

void TextGame::tryMove(const std::string& dir) {
    const auto& rooms = m_ctx.currentMap->getRooms();
    const Map::Room& cr = curRoom();
    int tx = cr.gridX, ty = cr.gridY;
    if (dir == "东") tx++;
    else if (dir == "西") tx--;
    else if (dir == "南") ty++;
    else if (dir == "北") ty--;
    else { appendLog("方向无效（东/西/南/北）。"); return; }

    int target = -1;
    for (int oi : cr.connectedRooms) {
        const auto& nr = rooms[oi];
        if (nr.gridX == tx && nr.gridY == ty) { target = oi; break; }
    }
    if (target < 0) { appendLog("那个方向没有相连的房间。"); return; }
    m_prevRoom = m_currentRoom;
    m_currentRoom = target;
    m_ctx.player->setPosition(rooms[target].getCenter());
    // 每次进入房间都在信息里完整描述房间情况（怪物/物品/NPC/楼梯/出口）
    logRoomDescription(target);
}

void TextGame::logRoomDescription(int roomIdx) {
    const auto& rooms = m_ctx.currentMap->getRooms();
    const Map::Room& room = rooms[roomIdx];
    appendLog("—— 你来到了【" + roomTypeName(room) + "】——");

    // 怪物
    bool hasMon = false;
    for (Monster* m : m_ctx.monsters)
        if (m->isAlive() && room.contains(m->getPosition())) {
            hasMon = true;
            appendLog("这里有怪物：" + monsterChineseName(m->getMonsterType()) + "（HP " +
                      std::to_string(m->getCurrentHp()) + "/" + std::to_string(m->getMaxHp()) +
                      "，攻 " + std::to_string(m->getAttack()) + "，防 " + std::to_string(m->getDefense()) + "）。");
        }
    if (hasMon) appendLog("可输入「攻击 名字」战斗一回合，或「逃跑」离开。");

    // 地面物品
    for (Item* it : m_ctx.itemsOnGround)
        if (room.contains(it->getPosition())) {
            if (it->getCategory() == ItemCategory::Gold)
                appendLog("地上有金币 ×" + std::to_string(it->getValue()) + "，可「拾取」。");
            else if (it->getCategory() == ItemCategory::Potion)
                appendLog("地上有一瓶治疗药水，可「拾取」。");
            else
                appendLog("地上有 " + it->getName() + "，可「拾取」。");
        }

    // NPC
    for (Npc* n : m_ctx.npcs)
        if (room.contains(n->getPosition())) {
            if (n->isMerchant()) appendLog("这里有商人，可「交易」买卖装备。");
            else {
                appendLog("这里有老向导，可「对话」听地牢故事。");
                appendLog(col(C_RED, "⚠ 你可以攻击向导！输入「攻击 向导」夺取强力资源（会改变结局）"));
            }
        }

    // 楼梯
    bool hasStairs = false;
    for (int x = room.topLeft.x; x <= room.bottomRight.x; ++x)
        for (int y = room.topLeft.y; y <= room.bottomRight.y; ++y)
            if (m_ctx.currentMap->getTile(x, y) == TileType::StairsDown) hasStairs = true;
    if (hasStairs) {
        if (m_ctx.currentLevel >= 5)
            appendLog("这里有通往深渊的门扉，但已被封死（第 5 层是终点）。");
        else
            appendLog("这里有向下的楼梯『>』！输入「下楼」前往下一层。");
    }

    // 出口
    std::vector<std::string> exits;
    for (int oi : room.connectedRooms) {
        const auto& nr = rooms[oi];
        std::string dir;
        if (nr.gridX == room.gridX + 1) dir = "东";
        else if (nr.gridX == room.gridX - 1) dir = "西";
        else if (nr.gridY == room.gridY + 1) dir = "南";
        else if (nr.gridY == room.gridY - 1) dir = "北";
        if (!dir.empty()) exits.push_back(dir + "→" + roomTypeName(nr));
    }
    if (!exits.empty()) {
        std::string e;
        for (size_t i = 0; i < exits.size(); ++i) {
            e += exits[i];
            if (i + 1 < exits.size()) e += "，";
        }
        appendLog("出口：" + e + "（输入「去 方向」移动）。");
    }
}

void TextGame::showHistory() {
    if (m_log.empty()) { appendLog("暂无历史信息。"); return; }
    std::cout << col(C_CYAN, "============ 历史信息（共 " + std::to_string(m_log.size()) + " 条）============") << "\n";
    for (size_t i = 0; i < m_log.size(); ++i)
        std::cout << i + 1 << ". " << m_log[i] << "\n";
    std::cout << col(C_CYAN, "===================================================") << "\n";
}

void TextGame::tryAttack(const std::string& target) {
    const Map::Room& room = curRoom();
    // 1) 先匹配怪物（空参数优先打房间内第一只活怪）
    Monster* pick = nullptr;
    for (Monster* m : m_ctx.monsters) {
        if (!m->isAlive() || !room.contains(m->getPosition())) continue;
        if (target.empty()) { pick = m; break; }
        if (monsterChineseName(m->getMonsterType()) == target ||
            m->getName() == target) { pick = m; break; }
    }
    if (pick) {
        // —— 与怪物战斗一回合（玩家先手）——
        CombatResult r = CombatSystem::playerAttackMonster(*m_ctx.player, *pick, m_ctx);
        appendLog(r.message);
        if (!r.targetKilled) {
            // 怪物反击
            CombatResult rr = CombatSystem::monsterAttackPlayer(*pick, *m_ctx.player, m_ctx);
            appendLog(rr.message);
            if (m_ctx.player->getCurrentHp() <= 0) {
                appendLog(col(C_RED, "你被击倒了……你的冒险结束了。"));
                m_ctx.state = GameContext::GameState::Dead;
            }
            return;
        }
        // —— 击杀魔王：按是否背叛向导判定结局 ——
        if (pick->getMonsterType() == MonsterType::Boss) {
            appendLog(col(C_RED, "【黑渊之眼】倒下了！"));
            // 魔王宝库照常掉落
            auto drops = DropSystem::generateDrops(*pick, pick->getPosition(), m_ctx);
            for (Item* it : drops) {
                if (it->getCategory() == ItemCategory::Gold) {
                    m_ctx.player->addGold(it->getValue());
                    appendLog("拾取了 金币 ×" + std::to_string(it->getValue()) + "。");
                    delete it;
                } else {
                    m_ctx.player->addItem(it);
                    appendLog("拾取了 " + it->getName() + "。");
                }
            }
            m_ctx.removeMonster(pick);
            delete pick;
            if (m_betrayedGuide) {
                // —— 背叛者结局：成为新的魔王 ——
                appendLog(col(C_YELLOW, "地牢深处传来低语：「你击败了它……可你的剑上还沾着向导的血。」"));
                appendLog(col(C_MAGENTA, "深渊王座认你为主——你——成为了新的魔王！"));
                m_ctx.state = GameContext::GameState::Victory;
            } else {
                // —— 勇者结局：地牢恢复和平 ——
                appendLog(col(C_YELLOW, "地牢恢复了和平！勇者凯旋！"));
                m_ctx.state = GameContext::GameState::Victory;
            }
            return;
        }
        // —— 普通怪物：掉落并自动拾取 ——
        auto drops = DropSystem::generateDrops(*pick, pick->getPosition(), m_ctx);
        for (Item* it : drops) {
            if (it->getCategory() == ItemCategory::Gold) {
                m_ctx.player->addGold(it->getValue());
                appendLog("拾取了 金币 ×" + std::to_string(it->getValue()) + "。");
                delete it;
            } else {
                m_ctx.player->addItem(it);
                appendLog("拾取了 " + it->getName() + "。");
            }
        }
        m_ctx.removeMonster(pick);
        delete pick;
        return;
    }

    // 2) 房间没有怪物（或目标不是怪物）→ 尝试攻击向导
    Npc* guide = nullptr;
    for (Npc* n : m_ctx.npcs)
        if (!n->isMerchant() && room.contains(n->getPosition())) { guide = n; break; }
    if (!guide) {
        appendLog(target.empty() ? "房间里没有怪物或向导可攻击。" : "房间里没有叫「" + target + "」的怪物或向导。");
        return;
    }
    // 目标名匹配向导（支持「向导」/ 向导全名 / 名字片段）
    if (!target.empty() && target != "向导" && target != "npc" &&
        guide->getName().find(target) == std::string::npos) {
        appendLog("房间里没有叫「" + target + "」的怪物或向导。");
        return;
    }
    tryAttackGuide(guide);
}

void TextGame::tryAttackGuide(Npc* guide) {
    // 向导血量：40 + 层×15（比同级怪物肉一些，补给充分即可击杀）
    int maxHp = 40 + m_ctx.currentLevel * 15;
    auto it = m_guideHp.find(guide);
    int hp = (it == m_guideHp.end()) ? maxHp : it->second;
    int dmg = CombatSystem::calculateDamage(m_ctx.player->getAttack(), 0);
    hp -= dmg;
    m_guideHp[guide] = hp;
    appendLog(col(C_RED, "你挥剑砍向【" + guide->getName() + "】！造成 " + std::to_string(dmg) + " 点伤害！"));
    if (hp <= 0) {
        // —— 向导倒下：夺取强力资源 ——
        appendLog(col(C_RED, "【" + guide->getName() + "】倒下了……"));
        int gold = 50 + m_ctx.currentLevel * 25;
        int exp = 80 + m_ctx.currentLevel * 40;
        m_ctx.player->addGold(gold);
        m_ctx.player->gainExp(exp);
        appendLog(col(C_ORANGE, "你从向导的遗物中搜刮出：金币 ×" + std::to_string(gold)
                  + "！经验 +" + std::to_string(exp) + "！"));
        // 强力装备：向导秘宝（攻/防加成随层数提升）
        Item* sword = new Item("向导秘宝剑", 'S', ItemCategory::Weapon, ItemRarity::Epic);
        sword->setAttackBonus(5 + m_ctx.currentLevel);
        sword->setValue(120 + m_ctx.currentLevel * 30);
        Item* armor = new Item("向导秘银甲", 'A', ItemCategory::Armor, ItemRarity::Epic);
        armor->setDefenseBonus(3 + m_ctx.currentLevel);
        armor->setValue(100 + m_ctx.currentLevel * 25);
        if (m_ctx.player->getInventorySize() + 2 <= 20) {
            m_ctx.player->addItem(sword);
            m_ctx.player->addItem(armor);
            appendLog(col(C_ORANGE, "获得强力装备：" + sword->getFullDescription()
                      + " 与 " + armor->getFullDescription()));
        } else if (m_ctx.player->getInventorySize() < 20) {
            m_ctx.player->addItem(sword);
            appendLog(col(C_ORANGE, "获得强力装备：" + sword->getFullDescription()));
            delete armor;
            appendLog("背包已满，「向导秘银甲」掉在了地上。（可先到商店卖掉旧装备）");
        } else {
            delete sword;
            delete armor;
            appendLog("背包已满，强力装备散落一地……（可先到商店卖掉旧装备）");
        }
        m_ctx.removeNpc(guide);
        m_guideHp.erase(guide);
        delete guide;
        // 背叛标记：击败魔王后将无法以勇者通关，而是成为新魔王
        m_betrayedGuide = true;
        appendLog(col(C_YELLOW, "向导之死传遍了地牢……再也没有人给你讲故事了。"));
        appendLog(col(C_RED, "你背叛了地牢的秩序。深渊正在注视你——击败魔王之日，就是深渊王座易主之时。"));
        return;
    }
    // 向导反击（伤害不高但会挨骂）
    int gdmg = CombatSystem::calculateDamage(3 + m_ctx.currentLevel, m_ctx.player->getDefense());
    m_ctx.player->takeDamage(gdmg);
    appendLog(col(C_YELLOW, guide->getName() + "：「你竟敢……！」") + " 向导反击，你受到 "
              + std::to_string(gdmg) + " 点伤害（向导 HP " + std::to_string(hp) + "/" + std::to_string(maxHp) + "）。");
    if (m_ctx.player->getCurrentHp() <= 0) {
        appendLog(col(C_RED, "你被击倒了……你的冒险结束了。"));
        m_ctx.state = GameContext::GameState::Dead;
    }
}

void TextGame::tryFlee() {
    const Map::Room& room = curRoom();
    bool hasMon = false;
    for (Monster* m : m_ctx.monsters)
        if (m->isAlive() && room.contains(m->getPosition())) { hasMon = true; break; }
    if (!hasMon) { appendLog("这里没有怪物，不需要逃跑。"); return; }
    if (m_prevRoom < 0 || m_prevRoom == m_currentRoom) { appendLog("无路可退！"); return; }
    // 逃跑代价：房间内随机一只怪物追打一下
    std::vector<Monster*> mons;
    for (Monster* m : m_ctx.monsters)
        if (m->isAlive() && room.contains(m->getPosition())) mons.push_back(m);
    if (!mons.empty()) {
        Monster* chaser = mons[rand() % mons.size()];
        CombatResult rr = CombatSystem::monsterAttackPlayer(*chaser, *m_ctx.player, m_ctx);
        appendLog("逃跑途中，" + monsterChineseName(chaser->getMonsterType()) + "追上来咬了一口！" + rr.message);
    }
    const auto& rooms = m_ctx.currentMap->getRooms();
    appendLog("你慌不择路，逃回了【" + roomTypeName(rooms[m_prevRoom]) + "】。");
    m_currentRoom = m_prevRoom;
    m_ctx.player->setPosition(rooms[m_currentRoom].getCenter());
    if (m_ctx.player->getCurrentHp() <= 0) {
        appendLog("你被击倒了……你的冒险结束了。");
        m_ctx.state = GameContext::GameState::Dead;
    }
}

void TextGame::tryOpenTreasure() {
    const Map::Room& room = curRoom();
    if (room.type != Map::RoomType::Treasure) {
        appendLog("这不是宝箱房，没有宝箱可开。");
        return;
    }
    std::vector<Item*> here;
    for (Item* it : m_ctx.itemsOnGround)
        if (room.contains(it->getPosition())) here.push_back(it);
    if (here.empty()) { appendLog("宝箱已经被打开过了，空空如也。"); return; }
    for (Item* it : here) {
        m_ctx.removeItemOnGround(it);
        if (it->getCategory() == ItemCategory::Gold) {
            m_ctx.player->addGold(it->getValue());
            appendLog("打开宝箱！获得 金币 ×" + std::to_string(it->getValue()) + "。");
            delete it;
        } else {
            m_ctx.player->addItem(it);
            appendLog("打开宝箱！获得 " + it->getName() + "。");
        }
    }
    appendLog("宝箱空了。");
}

void TextGame::tryTalk() {
    const Map::Room& room = curRoom();
    Npc* npc = nullptr;
    for (Npc* n : m_ctx.npcs)
        if (room.contains(n->getPosition())) { npc = n; break; }
    if (!npc) { appendLog("这个房间没有可对话的人。"); return; }
    if (npc->isMerchant()) {
        appendLog("商人：要交易的话输入「交易」吧。");
        return;
    }
    // 向导剧情对话
    appendLog(col(C_CYAN, "===== " + npc->getName() + " ====="));
    for (const auto& l : npc->getDialogue()) {
        appendLog(l);
    }
    appendLog(col(C_CYAN, "===================="));
}

void TextGame::tryTrade() {
    const Map::Room& room = curRoom();
    Npc* merchant = nullptr;
    for (Npc* n : m_ctx.npcs)
        if (n->isMerchant() && room.contains(n->getPosition())) { merchant = n; break; }
    if (!merchant) { appendLog("这个房间没有商人。"); return; }
    m_ctx.currentShop->generateShopItems(m_ctx.currentLevel, 6);
    m_trading = true;
    appendLog("商人：欢迎光临！看看我这的宝贝吧。");
}

void TextGame::tryPickup() {
    const Map::Room& room = curRoom();
    std::vector<Item*> here;
    for (Item* it : m_ctx.itemsOnGround)
        if (room.contains(it->getPosition())) here.push_back(it);
    if (here.empty()) { appendLog("脚下没有可以拾取的东西。"); return; }
    for (Item* it : here) {
        m_ctx.removeItemOnGround(it);
        if (it->getCategory() == ItemCategory::Gold) {
            m_ctx.player->addGold(it->getValue());
            appendLog("拾取了 金币 ×" + std::to_string(it->getValue()) + "。");
            delete it;
        } else {
            m_ctx.player->addItem(it);
            appendLog("拾取了 " + it->getName() + "。");
        }
    }
}

void TextGame::showInventory() {
    Player* p = m_ctx.player;
    if (p->getInventory().empty()) { appendLog("背包是空的。"); return; }
    appendLog(col(C_CYAN, "===== 背包 ====="));
    int i = 1;
    for (Item* it : p->getInventory()) {
        appendLog("[" + std::to_string(i) + "] " + it->getFullDescription());
        ++i;
    }
    appendLog("输入「使用 序号」使用/装备。");
}

void TextGame::tryUseItem(const std::string& arg) {
    if (arg.empty()) { appendLog("用法：使用 背包序号"); return; }
    int idx = atoi(arg.c_str());
    auto& inv = m_ctx.player->getInventory();
    if (idx < 1 || idx > static_cast<int>(inv.size())) { appendLog("没有这个背包序号。"); return; }
    Item* it = inv[idx - 1];
    std::string r = it->use(*m_ctx.player);
    appendLog(r);
    // 药水使用后消耗
    if (it->getCategory() == ItemCategory::Potion) {
        m_ctx.player->removeItem(it);
        delete it;
    }
}

void TextGame::tryDownStairs() {
    const Map::Room& room = curRoom();
    bool hasStairs = false;
    for (int x = room.topLeft.x; x <= room.bottomRight.x; ++x)
        for (int y = room.topLeft.y; y <= room.bottomRight.y; ++y)
            if (m_ctx.currentMap->getTile(x, y) == TileType::StairsDown) hasStairs = true;
    if (!hasStairs) { appendLog("这个房间没有楼梯。"); return; }
    if (m_ctx.currentLevel >= 5) {
        appendLog("这已是地牢最底层（第 5 层），没有更深的楼梯了。");
        appendLog("去击败魔王即可通关！");
        return;
    }
    nextLevel();
}

void TextGame::doWait() {
    appendLog("你屏息等待了一回合……");
    monsterCounterAttack();
}

void TextGame::monsterCounterAttack() {
    const Map::Room& room = curRoom();
    std::vector<Monster*> mons;
    for (Monster* m : m_ctx.monsters)
        if (m->isAlive() && room.contains(m->getPosition())) mons.push_back(m);
    if (mons.empty()) return;
    Monster* atk = mons[rand() % mons.size()];
    CombatResult rr = CombatSystem::monsterAttackPlayer(*atk, *m_ctx.player, m_ctx);
    appendLog(monsterChineseName(atk->getMonsterType()) + "扑了过来！" + rr.message);
    if (m_ctx.player->getCurrentHp() <= 0) {
        appendLog("你被击倒了……你的冒险结束了。");
        m_ctx.state = GameContext::GameState::Dead;
    }
}

void TextGame::printHelp() {
    appendLog(col(C_CYAN, "===== 帮助 ====="));
    appendLog("去 东/西/南/北 —— 移动到相邻房间");
    appendLog("攻击 [怪物名]  —— 与房间内怪物战斗一回合");
    appendLog("攻击 向导      —— " + col(C_RED, "攻击向导夺取强力资源（背叛者击败魔王后会成为新魔王）"));
    appendLog("逃跑           —— 逃回原房间（会被怪物追打一下）");
    appendLog("开宝箱         —— 打开宝箱房里的宝箱");
    appendLog("对话           —— 与向导/商人对话（剧情）");
    appendLog("交易           —— 在商店房买卖装备（买 N / 卖 N / 离开）");
    appendLog("拾取           —— 捡起地上的物品");
    appendLog("背包           —— 查看背包（使用 序号）");
    appendLog("下楼           —— 在楼梯房前往下一层");
    appendLog("等待           —— 原地等待一回合");
    appendLog("查看           —— 重新查看当前房间");
    appendLog("历史           —— 查看全部历史信息（旧信息不会消失）");
    appendLog("帮助           —— 显示本帮助");
    appendLog("退出           —— 退出游戏");
}

// ==================== 主循环 ====================
void TextGame::run() {
    newGame();
    std::string line;
    while (m_running) {
        if (m_ctx.state == GameContext::GameState::Victory) {
            if (m_betrayedGuide) {
                // —— 背叛者结局：成为新的魔王 ——
                std::cout << "\n" << col(C_MAGENTA, "========== 新 魔 王 诞 生 ==========") << "\n";
                std::cout << col(C_MAGENTA, "你坐在深渊王座上，地牢的黑暗向你俯首。\n");
                std::cout << col(C_RED, "曾经屠龙的勇者，最终成了新的恶龙。\n");
                std::cout << col(C_YELLOW, "向导的血让你登上了王座——从此地牢有了新的传说。\n");
                std::cout << col(C_CYAN, "按 N 重新挑战（从勇者再来），Q 退出。\n");
            } else {
                // —— 勇者结局：地牢恢复和平 ——
                std::cout << "\n" << col(C_CYAN, "========== 胜 利 ==========") << "\n";
                std::cout << "你击败了魔王，地牢恢复了和平！\n";
                std::cout << "勇者的传说被写进了史书，受世人敬仰。\n";
                std::cout << "按 N 重新挑战，Q 退出。\n";
            }
            std::string c;
            std::getline(std::cin, c);
            if (toLower(trim(c)) == "n") { newGame(); continue; }
            break;
        }
        if (m_ctx.state == GameContext::GameState::Dead) {
            std::cout << "\n" << col(C_RED, "========== 你倒下了 ==========") << "\n";
            std::cout << "你的冒险到此结束。按 N 重新开始，Q 退出。\n";
            std::string c;
            std::getline(std::cin, c);
            if (toLower(trim(c)) == "n") { newGame(); continue; }
            break;
        }
        renderFrame();
        if (m_trading) {
            renderTrade(std::cout);
        }
        std::cout << "请输入指令 > ";
        std::cout.flush();
        std::getline(std::cin, line);
        handleCommand(line);
    }
    cleanup();
}

} // namespace dq
