#pragma once
#include <stdint.h>
#include <string>
#include "item_selectable.h"
#include "config.h"

static uint32_t PsvKeys[] = {
    SCE_CTRL_PSBUTTON,
    SCE_CTRL_UP,
    SCE_CTRL_DOWN,
    SCE_CTRL_LEFT,
    SCE_CTRL_RIGHT,
    SCE_CTRL_CROSS,
    SCE_CTRL_TRIANGLE,
    SCE_CTRL_CIRCLE,
    SCE_CTRL_SQUARE,
    SCE_CTRL_SELECT,
    SCE_CTRL_START,
    SCE_CTRL_L1,
    SCE_CTRL_R1,
    SCE_CTRL_L2,
    SCE_CTRL_R2,
    SCE_CTRL_L3,
    SCE_CTRL_R3,
    SCE_CTRL_LSTICK_UP,
    SCE_CTRL_LSTICK_DOWN,
    SCE_CTRL_LSTICK_LEFT,
    SCE_CTRL_LSTICK_RIGHT,
    SCE_CTRL_RSTICK_UP,
    SCE_CTRL_RSTICK_DOWN,
    SCE_CTRL_RSTICK_LEFT,
    SCE_CTRL_RSTICK_RIGHT,
};

#define PSV_KEYS_SIZE (sizeof(PsvKeys) / sizeof(PsvKeys[0]))

class ItemHotkey : public ItemSelectable
{
public:
    ItemHotkey(HotKeyConfig index, uint32_t *hotkey)
        : ItemSelectable(index + HOTKEY_SAVESTATE),
          _hotkey(hotkey){};

    virtual ~ItemHotkey(){};

    void Show(bool selected) override
    {
        const char *text = GetText();
        ImGui::Selectable(text, selected);
        ImGui::NextColumn();

        bool is_popup = ImGui::IsPopupOpen(text);

        if (_actived && !is_popup)
        {
            ImGui::OpenPopup(text);
        }

        if (My_Imgui_BeginCombo(text, _GetPreviewText(), ImGuiComboFlags_NoArrowButton))
        {
            if (!_actived && is_popup)
            {
                ImGui::CloseCurrentPopup();
            }
            for (size_t i = 0; i < _GetTotalCount(); i++)
            {
                bool enabled = PsvKeys[i] & *_hotkey;
                if (!enabled)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 100, 255));
                }

                ImGui::Selectable(_GetOptionString(i), i == _GetIndex());
                if (i == _GetIndex() && ImGui::GetScrollMaxY() > 0.f)
                {
                    ImGui::SetScrollHereY((float)i / (float)PSV_KEYS_SIZE);
                }

                if (!enabled)
                {
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndCombo();
        }
    };

private:
    size_t _GetTotalCount() override
    {
        return PSV_KEYS_SIZE;
    };

    const char *_GetPreviewText() override
    {
        static uint32_t old_hotkey = 0;
        static std::string s;
        if (old_hotkey == *_hotkey)
        {
            return s.c_str();
        }

        old_hotkey = *_hotkey;
        s.clear();
        for (size_t i = 0; i < PSV_KEYS_SIZE; i++)
        {
            uint32_t key = PsvKeys[i];
            if (key & *_hotkey)
            {
                if (s.size() == 0)
                {
                    s = TEXT(Config::ControlTextMap.at(key));
                }
                else
                {
                    s += std::string(" + ") + TEXT(Config::ControlTextMap.at(key));
                }
            }
        }

        return s.c_str();
    };

    const char *_GetOptionString(size_t index) override
    {
        return TEXT(Config::ControlTextMap.at(PsvKeys[index]));
    };

    void _OnClick(Input *input) override
    {
        uint32_t k = PsvKeys[_index];
        if (*_hotkey & k)
        {
            *_hotkey &= ~k;
        }
        else
        {
            *_hotkey |= k;
        }
    };

    uint32_t *_hotkey;
};