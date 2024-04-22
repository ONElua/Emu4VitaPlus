#include "config_item.h"
#include "my_imgui.h"

ConfigItem::ConfigItem(size_t text_id, size_t *config, size_t sizeof_config, size_t config_text_start, size_t config_count)
    : ItemBase(text_id),
      _config(config),
      _config_mask((1 << (sizeof_config * 8)) - 1),
      _config_text_start(config_text_start),
      _config_count(config_count),
      _actived(false)
{
}

ConfigItem::~ConfigItem()
{
}

void ConfigItem::Show(bool selected)
{
    ImGui::Selectable(TEXT(_text_id), selected);
    ImGui::NextColumn();

    bool is_popup = ImGui::IsPopupOpen(TEXT(_text_id));

    if (_actived && !is_popup)
    {
        ImGui::OpenPopup(TEXT(_text_id));
    }

    if (MyBeginCombo(TEXT(_text_id), TEXT(_config_text_start + GetConfig()), ImGuiComboFlags_NoArrowButton))
    {
        if (!_actived && is_popup)
        {
            ImGui::CloseCurrentPopup();
        }
        for (size_t i = 0; i < _config_count; i++)
        {
            ImGui::Selectable(TEXT(_config_text_start + i), GetConfig() == i);
            if (GetConfig() == i)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::NextColumn();
}

void ConfigItem::OnActive(Input *input)
{
    _actived = true;
    _old_config = GetConfig();
    input->PushCallbacks();
    SetInputHooks(input);
}

void ConfigItem::_OnKeyUp(Input *input)
{
    // LogDebug("_OnKeyUp %d %d", GetConfig(), _config_count);
    size_t config = GetConfig();
    if (config == 0)
    {
        SetConfig(_config_count - 1);
    }
    else
    {
        SetConfig(config - 1);
    }
}

void ConfigItem::_OnKeyDown(Input *input)
{
    // LogDebug("_OnKeyDown %d %d", *_config, _config_count);
    size_t config = GetConfig();
    if (config == _config_count - 1)
    {
        SetConfig(0);
    }
    else
    {
        SetConfig(config + 1);
    }
}

void ConfigItem::_OnClick(Input *input)
{
    LogFunctionName;
    _actived = false;
    input->PopCallbacks();
}

void ConfigItem::_OnCancel(Input *input)
{
    LogFunctionName;
    _actived = false;
    SetConfig(_old_config);
    input->PopCallbacks();
}

void ConfigItem::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&ConfigItem::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&ConfigItem::_OnKeyDown, this, input), true);
    input->SetKeyUpCallback(SCE_CTRL_CIRCLE, std::bind(&ConfigItem::_OnClick, this, input));
    input->SetKeyUpCallback(SCE_CTRL_CROSS, std::bind(&ConfigItem::_OnCancel, this, input));
}

void ConfigItem::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    input->UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
    input->UnsetKeyUpCallback(SCE_CTRL_CROSS);
}