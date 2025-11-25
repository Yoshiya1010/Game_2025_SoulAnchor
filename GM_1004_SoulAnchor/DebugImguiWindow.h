#pragma once
#include <string>
#include <vector>
#include <functional>
#include"main.h"

class DebugImguiWindow
{
public:

    //Å@LabelÇ∆ílÇéùÇΩÇπÇÈ
    struct Entry {
        std::string label;
        std::function<void()> drawFunc;
        bool deleteFlag;
    };

    static void Add(const std::string& label, std::function<void()> drawFunc,bool deleteFlag);
    static void Draw(); // ImGuië§Ç≈Ç‹Ç∆ÇﬂÇƒï`âÊ


    static void DebugFloat(const std::string& label, float value,bool deleteFlag=true)
    {
        DebugImguiWindow::Add(label, [value]() {
            ImGui::Text("%.6f", value);
            },
            deleteFlag);
    }



    static void DebugVector3(const std::string& label, Vector3* v, bool deleteFlag = true)
    {
        DebugImguiWindow::Add(label, [v]() {
            ImGui::Text("(%.2f, %.2f, %.2f)", v->x, v->y, v->z);
            },
            deleteFlag);
    }

    static void DebugString(const std::string& label, std::string* s, bool deleteFlag = true)
    {
        DebugImguiWindow::Add(label, [s]() {
            ImGui::Text("%s", s->c_str());
            },
            deleteFlag);
    }


    static void DebugBool(const std::string& label, bool* flag, bool deleteFlag = true)
    {
        DebugImguiWindow::Add(
            label,
            [flag]() {
                ImGui::Text("%s", (*flag ? "true" : "false"));
            },
            deleteFlag
        );
    }
private:
    static std::vector<Entry> entries;
};
