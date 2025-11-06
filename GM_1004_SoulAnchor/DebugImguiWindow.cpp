#include"DebugImguiWindow.h"

#include "imgui.h"

std::vector<DebugImguiWindow::Entry> DebugImguiWindow::entries;

void DebugImguiWindow::Add(const std::string& label, std::function<void()> drawFunc)
{
    entries.push_back({ label, drawFunc });//構造体をAddして配列に追加する
}

void DebugImguiWindow::Draw()
{
    //何もチェック事項がなければはじく
    if (entries.size() < 1) { return; };

    ImGui::Begin("Debug Inspector");

    for (auto& e : entries)//構造体分呼ばれる
    {
        ImGui::Text("%s", e.label.c_str());
        ImGui::SameLine();
        e.drawFunc();//型に応じた描画関数が呼ばれる
    }

    ImGui::End();

    entries.clear();
}