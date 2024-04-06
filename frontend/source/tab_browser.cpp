#include "global.h"
#include "tab_browser.h"
#include "log.h"

TabBrowser::TabBrowser(Input *input, const char *path) : TabBase(input), _index(0)
{
    LogFunctionName;
    _directory = new Directory(path, gEmulator->GetValidExtensions());
}

TabBrowser::~TabBrowser()
{
    LogFunctionName;
    delete _directory;
}

void TabBrowser::SetInputHooks()
{
    _input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&TabBrowser::_OnKeyUp, this), true);
    _input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&TabBrowser::_OnKeyDown, this), true);
    _input->SetKeyUpCallback(SCE_CTRL_CIRCLE, std::bind(&TabBrowser::_OnKeyCircle, this));
    _input->SetKeyUpCallback(SCE_CTRL_CROSS, std::bind(&TabBrowser::_OnKeyCross, this));
}

void TabBrowser::UnsetInputHooks()
{
    _input->UnsetKeyDownCallback(SCE_CTRL_UP);
    _input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    _input->UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
    _input->UnsetKeyUpCallback(SCE_CTRL_CROSS);
}

void TabBrowser::Show(bool selected)
{
    if (ImGui::BeginTabItem("Browser", NULL, selected ? ImGuiTabItemFlags_SetSelected : 0))
    {
        ImGui::Text(_directory->GetCurrentPath().c_str());
        auto size = ImGui::GetContentRegionAvail();
        ImGui::ListBoxHeader("", {size.x * 0.5f, size.y});

        for (size_t i = 0; i < _directory->GetSize(); i++)
        {
            const auto item = _directory->GetItem(i);

            if (item.isDir)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            }

            ImGui::Selectable(item.name.c_str(), i == _index);
            if (item.isDir)
            {
                ImGui::PopStyleColor();
            }

            if (i == _index && ImGui::GetScrollMaxY() > 0.f)
            {
                ImGui::SetScrollHereY((float)_index / (float)_directory->GetSize());
            }
        }

        // LogDebug("GetScrollY %f %f", ImGui::GetScrollY(), ImGui::GetScrollMaxY());
        ImGui::ListBoxFooter();
        ImGui::EndTabItem();
    }
}

void TabBrowser::_OnKeyUp()
{
    _index += _directory->GetSize() - 1;
    _index %= _directory->GetSize();
}

void TabBrowser::_OnKeyDown()
{
    _index++;
    _index %= _directory->GetSize();
}

void TabBrowser::_OnKeyCircle()
{
    auto item = _directory->GetItem(_index);

    if (item.isDir)
    {
        _directory->SetCurrentPath(_directory->GetCurrentPath() + "/" + item.name);
        _index = 0;
    }
    else
    {
        if (gEmulator->LoadGame((_directory->GetCurrentPath() + "/" + item.name).c_str()))
        {
            gStatus = APP_STATUS_RUN_GAME;
        }
    }
}

void TabBrowser::_OnKeyCross()
{
    auto path = _directory->GetCurrentPath();
    if (path.size() <= 5)
    {
        return;
    }

    size_t pos = path.rfind('/');
    if (pos != std::string::npos)
    {
        LogDebug(path.c_str());
        path = path.substr(0, pos);
        LogDebug(path.c_str());
        _directory->SetCurrentPath(path);
    }
}