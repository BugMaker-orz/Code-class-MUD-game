#pragma once
#include "core/GameContext.h"
#include <string>
#include <vector>
#include <map>
namespace dq {
// ============================================================
// 纯文字版 MUD（版本二）
// ------------------------------------------------------------
// 不使用二维平面地图，改用命令式文字交互：
//   - 每次显示：当前层小地图（房间拓扑）+ 当前房间文字描述 + 信息 + 状态
//   - 指令：去 东/西/南/北 | 攻击 [怪物] | 逃跑 | 开宝箱 | 对话 | 交易
//           拾取 | 背包 | 使用 N | 下楼 | 等待 | 查看 | 帮助 | 退出
// 二维渲染代码（Renderer / InputManager）不参与本模式编译，
// 因此“用不到的二维地图代码”在本版本中整体不链接。
// ============================================================
class TextGame {
public:
    TextGame();
    ~TextGame();
    void run();
private:
    GameContext m_ctx;
    int m_currentRoom = -1;      // 玩家当前房间下标（rooms 中）
    int m_prevRoom = -1;         // 上一房间下标（用于“逃跑”）
    bool m_running = true;
    bool m_trading = false;      // 是否处于交易子界面
    bool m_betrayedGuide = false; // 是否攻击并击杀了向导（背叛）：击败魔王后将成为新魔王
    std::vector<std::string> m_log;        // 信息日志（保留最近若干条）
    std::map<Npc*, int> m_guideHp;         // 向导剩余血量（攻击向导特性用）

    // 游戏流程
    void newGame();
    void generateLevel();
    void nextLevel();
    void clearLevelEntities();
    void cleanup();

    // 渲染
    void renderFrame();
    void renderMiniMap(std::ostream& out);
    void renderRoomDesc(std::ostream& out);
    void renderStatus(std::ostream& out);
    void renderLog(std::ostream& out);
    void renderTrade(std::ostream& out);

    // 命令
    void handleCommand(const std::string& line);
    void printHelp();

    // 房间动作
    const Map::Room& curRoom();
    std::string roomTypeName(const Map::Room& r) const;
    std::string monsterChineseName(MonsterType t) const;
    void tryMove(const std::string& dir);
    void tryAttack(const std::string& target);
    void tryAttackGuide(Npc* guide);   // 攻击向导：夺取强力资源
    void tryFlee();
    void tryOpenTreasure();
    void tryTalk();
    void tryTrade();
    void tryPickup();
    void showInventory();
    void tryUseItem(const std::string& arg);
    void tryDownStairs();
    void doWait();
    void monsterCounterAttack();   // 房间内随机一只活怪攻击玩家一次
    void logRoomDescription(int roomIdx);   // 进入房间时把完整描述写入信息日志
    void showHistory();                     // 显示全部历史信息

    void appendLog(const std::string& msg);
    static std::string trim(const std::string& s);
    static std::string toLower(const std::string& s);
    static std::string col(const std::string& code, const std::string& text);  // ANSI 上色
};
}
