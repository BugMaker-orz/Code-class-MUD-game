#pragma once
#include "core/Entity.h"
#include <string>
#include <vector>
namespace dq {
class Npc : public Entity {
public:
    enum class NpcType { Guide, Merchant, Elder };
    Npc(const std::string& name, char symbol, const Position& pos, NpcType type);
    NpcType getNpcType() const { return m_type; }
    bool isMerchant() const { return m_type == NpcType::Merchant; }
    const std::vector<std::string>& getDialogue() const { return m_dialogue; }
    void setDialogue(const std::vector<std::string>& lines) { m_dialogue = lines; }
    std::string getDescription() const override;
    // 按楼层返回向导/商人的名字与台词（剧情逐层推进 + 整活风格）
    static std::string guideName(int lvl);
    static std::vector<std::string> guideDialogue(int lvl);
    static std::vector<std::string> merchantDialogue(int lvl);
private:
    NpcType m_type;
    std::vector<std::string> m_dialogue;
};
}
