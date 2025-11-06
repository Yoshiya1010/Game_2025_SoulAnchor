#pragma once
#include <string>
#include <vector>
#include <functional>
#include"main.h"

class DebugImguiWindow
{
public:

    //@Label‚Æ’l‚ğ‚½‚¹‚é
    struct Entry {
        std::string label;
        std::function<void()> drawFunc;
    };

    static void Add(const std::string& label, std::function<void()> drawFunc);
    static void Draw(); // ImGui‘¤‚Å‚Ü‚Æ‚ß‚Ä•`‰æ


    static void DebugFloat(const std::string& label, float* value)
    {
        DebugImguiWindow::Add(label, [value]() {
            ImGui::Text("%.3f", *value);
            });
    }
    static void DebugVector3(const std::string& label, Vector3* v)
    {
        DebugImguiWindow::Add(label, [v]() {
            ImGui::Text("(%.2f, %.2f, %.2f)", v->x, v->y, v->z);
            });
    }

    static void DebugString(const std::string& label, std::string* s)
    {
        DebugImguiWindow::Add(label, [s]() {
            ImGui::Text("%s", s->c_str());
            });
    }
private:
    static std::vector<Entry> entries;
};
