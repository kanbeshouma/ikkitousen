#pragma once

#ifdef Telecommunications
#include<string>
#include "../External/imgui/imgui.h"

namespace ImGui
{
    inline bool InputIntAbove(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"をつけるとGUI上で非表示になる
        return  ImGui::InputInt(hidden_name.c_str(), v, step, step_fast, flags);
    }
    inline bool DragFloatAbove(const char* label, float* v, float v_speed = 1.0f,
        float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"をつけるとGUI上で非表示になる
        return  ImGui::DragFloat(hidden_name.c_str(), v, v_speed, v_min, v_max, format, flags);
    }
    inline bool DragFloat2Above(const char* label, float v[2], float v_speed = 1.0f,
        float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"をつけるとGUI上で非表示になる
        return  ImGui::DragFloat2(hidden_name.c_str(), v, v_speed, v_min, v_max, format, flags);
    }
    inline bool DragFloat3Above(const char* label, float v[3], float v_speed = 1.0f,
        float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"をつけるとGUI上で非表示になる
        return  ImGui::DragFloat3(hidden_name.c_str(), v, v_speed, v_min, v_max, format, flags);
    }
    inline bool ColorEdit4Above(const char* label, float col[4], ImGuiColorEditFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"をつけるとGUI上で非表示になる
        return  ImGui::ColorEdit4(hidden_name.c_str(), col, flags);
    }

    inline bool InputTextAbove(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"をつけるとGUI上で非表示になる
        return ImGui::InputText(hidden_name.c_str(), buf, buf_size, flags, callback, user_data);
    }

    inline bool CheckboxAbove(const char* label, bool* v)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"をつけるとGUI上で非表示になる
        return ImGui::Checkbox(hidden_name.c_str(), v);
    }

};
#endif