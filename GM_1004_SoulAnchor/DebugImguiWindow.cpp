#include"DebugImguiWindow.h"

#include "imgui.h"

std::vector<DebugImguiWindow::Entry> DebugImguiWindow::entries;

void DebugImguiWindow::Add(const std::string& label, std::function<void()> drawFunc)
{
    entries.push_back({ label, drawFunc });//\‘¢‘Ì‚ğAdd‚µ‚Ä”z—ñ‚É’Ç‰Á‚·‚é
}

void DebugImguiWindow::Draw()
{
    ImGui::Begin("Debug Inspector");

    for (auto& e : entries)//\‘¢‘Ì•ªŒÄ‚Î‚ê‚é
    {
        ImGui::Text("%s", e.label.c_str());
        ImGui::SameLine();
        e.drawFunc();//Œ^‚É‰‚¶‚½•`‰æŠÖ”‚ªŒÄ‚Î‚ê‚é
    }

    ImGui::End();

    entries.clear();
}