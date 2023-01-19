#pragma once

#ifdef Telecommunications
#include<string>
#include "../External/imgui/imgui.h"

namespace ImGui
{
    inline bool InputIntAbove(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"�������GUI��Ŕ�\���ɂȂ�
        return  ImGui::InputInt(hidden_name.c_str(), v, step, step_fast, flags);
    }
    inline bool DragFloatAbove(const char* label, float* v, float v_speed = 1.0f,
        float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"�������GUI��Ŕ�\���ɂȂ�
        return  ImGui::DragFloat(hidden_name.c_str(), v, v_speed, v_min, v_max, format, flags);
    }
    inline bool DragFloat2Above(const char* label, float v[2], float v_speed = 1.0f,
        float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"�������GUI��Ŕ�\���ɂȂ�
        return  ImGui::DragFloat2(hidden_name.c_str(), v, v_speed, v_min, v_max, format, flags);
    }
    inline bool DragFloat3Above(const char* label, float v[3], float v_speed = 1.0f,
        float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"�������GUI��Ŕ�\���ɂȂ�
        return  ImGui::DragFloat3(hidden_name.c_str(), v, v_speed, v_min, v_max, format, flags);
    }
    inline bool ColorEdit4Above(const char* label, float col[4], ImGuiColorEditFlags flags = 0)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"�������GUI��Ŕ�\���ɂȂ�
        return  ImGui::ColorEdit4(hidden_name.c_str(), col, flags);
    }

    inline bool InputTextAbove(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"�������GUI��Ŕ�\���ɂȂ�
        return ImGui::InputText(hidden_name.c_str(), buf, buf_size, flags, callback, user_data);
    }

    inline bool CheckboxAbove(const char* label, bool* v)
    {
        ImGui::Text(label);
        std::string hidden_name = "##" + std::string(label); // "##"�������GUI��Ŕ�\���ɂȂ�
        return ImGui::Checkbox(hidden_name.c_str(), v);
    }



    //�C�ӂ̒l���O���t�ɋL�^����
    struct ValuePlotter final
    {
    public:
        ValuePlotter(float interval = 0.2f) : interval(interval) {}

        //<�O���t�����֐�>//
        //������ : elapsed_time
        //������ : �O���t�̖��O
        //��O���� : ���ۂɋL�^����l
        //��l���� : �O���t�̍Œ�l
        //��܈��� : �O���t�̍ō��l
        //��Z���� : �O���t�̒��̖��O(printf�݂����ȏ�����)
        void record(float elapsed_time, const char* plot_label, float value,
            float scale_min, float scale_max, char const* overlay_label, ...)
        {
            refresh_time += elapsed_time;
            if (static_cast<int>(refresh_time / interval) >= 1)
            {
                values_offset = values_offset >= IM_ARRAYSIZE(values) ? 0 : values_offset;
                values[values_offset] = value;

                va_list arg_list;
                va_start(arg_list, overlay_label);
                vsprintf_s(overlay, overlay_label, arg_list);
                va_end(arg_list);

                ++values_offset;
                refresh_time = 0;
            }

            ImGui::Begin("plots");

            ImGui::Text(plot_label);
            std::string hidden_name = "##" + std::string(plot_label);
            ImGui::PlotLines(hidden_name.c_str(), values, IM_ARRAYSIZE(values), values_offset, overlay,
                scale_min, scale_max, ImVec2(ImGui::GetWindowSize().x * 0.9f, 90.0f));

            ImGui::End();
        }

    private:
        float values[90] = {};
        char overlay[32] = {};
        int values_offset = 0;
        float refresh_time = 0.0f;
        float interval = 0.0f;
    };


};





#endif