#include <vita2d.h>
#include <string>
#include <math.h>
#include "tab_browser.h"
#include "emulator.h"
#include "video.h"
#include "app.h"
#include "ui.h"
#include "file.h"
#include "log.h"
#include "favorite.h"
#include "config.h"
#include "icons.h"
#include "state_manager.h"
#include "misc.h"
#include "utils.h"

TabBrowser::TabBrowser() : TabSeletable(TAB_BROWSER),
                           _texture(nullptr),
                           _texture_max_width(BROWSER_TEXTURE_MAX_WIDTH),
                           _texture_max_height(BROWSER_TEXTURE_MAX_HEIGHT),
                           _in_refreshing(false)
{
    LogFunctionName;

    const char *path = gConfig->last_rom.c_str();
    _directory = new Directory(File::GetDir(path).c_str(), gEmulator->GetValidExtensions());
    std::string name = File::GetName(path);
    for (size_t i = 0; i < _directory->GetSize(); i++)
    {
        if (_directory->GetItemName(i) == name)
        {
            _index = i;
            break;
        }
    }
    _UpdateStatus();
    _UpdateTexture();
}

TabBrowser::~TabBrowser()
{
    LogFunctionName;
    delete _directory;
}

void TabBrowser::SetInputHooks(Input *input)
{
    TabSeletable::SetInputHooks(input);
    input->SetKeyUpCallback(SCE_CTRL_LEFT, std::bind(&TabBrowser::_OnKeyLeft, this, input));
    input->SetKeyUpCallback(SCE_CTRL_RIGHT, std::bind(&TabBrowser::_OnKeyRight, this, input));
    input->SetKeyUpCallback(CancelButton, std::bind(&TabBrowser::_OnKeyCross, this, input));
    input->SetKeyUpCallback(SCE_CTRL_START, std::bind(&TabBrowser::_OnKeyStart, this, input));
    // input->SetKeyUpCallback(SCE_CTRL_TRIANGLE, std::bind(&TabBrowser::_OnKeyTriangle, this, input));
    // input->SetKeyUpCallback(SCE_CTRL_SQUARE, std::bind(&TabBrowser::_OnKeySquare, this, input));
}

void TabBrowser::UnsetInputHooks(Input *input)
{
    TabSeletable::UnsetInputHooks(input);
    input->UnsetKeyUpCallback(SCE_CTRL_LEFT);
    input->UnsetKeyUpCallback(SCE_CTRL_RIGHT);
    input->UnsetKeyUpCallback(CancelButton);
    input->UnsetKeyUpCallback(SCE_CTRL_START);
    // input->UnsetKeyUpCallback(SCE_CTRL_TRIANGLE);
    // input->UnsetKeyUpCallback(SCE_CTRL_SQUARE);
}

void TabBrowser::Show(bool selected)
{
    std::string title = std::string(TAB_ICONS[_title_id]) + TEXT(_title_id);
    if (ImGui::BeginTabItem(title.c_str(), NULL, selected ? ImGuiTabItemFlags_SetSelected : 0))
    {
        ImVec2 size = {0.f, 0.f};
        if (_status_text.size() > 0)
        {
            ImVec2 s = ImGui::CalcTextSize(_status_text.c_str());
            size.y = -s.y * (s.x / ImGui::GetContentRegionAvailWidth() + 1);
        }

        if (ImGui::BeginChild(TEXT(_title_id), size))
        {
            ImGui::Columns(2, NULL, false);

            std::string current_path = _directory->GetCurrentPath();
            if (_directory->GetSearchString().size() > 0 && _directory->GetSearchResults().size() > 0)
            {
                current_path += std::string(" " ICON_SERACH) + _directory->GetSearchString();
            }
            ImGui::Text(current_path.c_str());
            if (ImGui::ListBoxHeader("", ImGui::GetContentRegionAvail()))
            {
                if (_in_refreshing)
                {
                    _spin_text.Show();
                }
                else
                {
                    for (size_t i = 0; i < _directory->GetSize(); i++)
                    {
                        const DirItem &item = _directory->GetItem(i);

                        std::string name(item.name);
                        if (!item.is_dir)
                        {

                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                        }

                        if (gFavorites->find(name) != gFavorites->end())
                        {
                            name.insert(0, ICON_EMPTY_STAR_SPACE);
                        }

                        if (i == _index)
                        {
                            My_Imgui_Selectable(name.c_str(), true, &_moving_status);
                        }
                        else
                        {
                            ImGui::Selectable(name.c_str());
                        }

                        if (!item.is_dir)
                        {
                            ImGui::PopStyleColor();
                        }

                        if (i == _index && ImGui::GetScrollMaxY() > 0.f)
                        {
                            ImGui::SetScrollHereY((float)_index / (float)_directory->GetSize());
                        }
                    }
                }

                ImGui::ListBoxFooter();
            }
            ImGui::NextColumn();
            ImVec2 avail_size = ImGui::GetContentRegionAvail();
            _texture_max_width = avail_size.x;
            _texture_max_height = avail_size.y;

            if (_texture != nullptr)
            {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                pos.x += ceilf(fmax(0.0f, (avail_size.x - _texture_width) * 0.5f));
                pos.y += ceilf(fmax(0.0f, (avail_size.y - _texture_height) * 0.5f));
                ImGui::SetCursorScreenPos(pos);
                ImGui::Image(_texture, {_texture_width, _texture_height});
            }

            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        ImGui::EndChild();

        if (_status_text.size() > 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xcc, 0xcc, 0xcc, 255));
            ImGui::TextWrapped(_status_text.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndTabItem();
    }
}

void TabBrowser::_OnActive(Input *input)
{
    LogFunctionName;
    if (_index >= _directory->GetSize())
    {
        return;
    }

    auto item = _directory->GetItem(_index);

    if (item.is_dir)
    {
        _in_refreshing = true;
        LogDebug("%d %s", _directory->GetCurrentPath().size(), item.name.c_str());
        if (_directory->GetCurrentPath().size() == 0)
        {
            _directory->SetCurrentPath(item.name);
        }
        else
        {
            _directory->SetCurrentPath(_directory->GetCurrentPath() + "/" + item.name);
        }
        _in_refreshing = false;

        _index = 0;
        _UpdateStatus();
    }
    else
    {
        if (gEmulator->LoadRom((_directory->GetCurrentPath() + "/" + item.name).c_str(), item.entry_name.c_str(), item.crc32))
        {
            UnsetInputHooks(input);
        }
    }
}

void TabBrowser::_OnKeyCross(Input *input)
{
    auto path = _directory->GetCurrentPath();

    _in_refreshing = true;
    if (path.size() <= 5)
    {
        _directory->SetCurrentPath("");
    }
    else
    {
        size_t pos = path.rfind('/');
        if (pos != std::string::npos)
        {
            _directory->SetCurrentPath(path.substr(0, pos));
        }
    }

    _in_refreshing = false;

    _index = 0;
    _UpdateStatus();
}

void TabBrowser::_OnKeyStart(Input *input)
{
    if (_index >= _directory->GetSize())
    {
        return;
    }

    DirItem item = _directory->GetItem(_index);
    if (item.is_dir)
    {
        return;
    }

    const auto &iter = gFavorites->find(item.name);
    if (iter == gFavorites->end())
    {
        gFavorites->emplace(item.name, Favorite{item, _directory->GetCurrentPath()});
    }
    else
    {
        gFavorites->erase(item.name);
    }

    gFavorites->Save();
    _UpdateStatus();
}

void TabBrowser::_OnKeyUp(Input *input)
{
    TabSeletable::_OnKeyUp(input);
    _UpdateStatus();
    _UpdateTexture();
}

void TabBrowser::_OnKeyDown(Input *input)
{
    TabSeletable::_OnKeyDown(input);
    _UpdateStatus();
    _UpdateTexture();
}

void TabBrowser::_OnKeyLeft(Input *input)
{
    if (_index <= 10)
    {
        _index = 0;
    }
    else
    {
        _index -= 10;
    }

    _moving_status.Reset();
    _UpdateStatus();
    _UpdateTexture();
}

void TabBrowser::_OnKeyRight(Input *input)
{
    _index += 10;
    if (_index >= _GetItemCount())
    {
        _index = _GetItemCount() - 1;
    }

    _moving_status.Reset();
    _UpdateStatus();
    _UpdateTexture();
}

void TabBrowser::_UpdateTexture()
{
    // LogFunctionName;

    if (_texture != nullptr)
    {
        gVideo->Lock();
        vita2d_wait_rendering_done();
        vita2d_free_texture(_texture);
        _texture = nullptr;
        gVideo->Unlock();
    }

    if (_index >= _directory->GetSize())
    {
        return;
    }

    const DirItem &item = _directory->GetItem(_index);
    if (item.is_dir)
    {
        return;
    }

    _texture = GetRomPreviewImage(_directory->GetCurrentPath().c_str(), item.name.c_str());

    if (_texture)
    {
        CalcFitSize(vita2d_texture_get_width(_texture),
                    vita2d_texture_get_height(_texture),
                    _texture_max_width,
                    _texture_max_height,
                    &_texture_width,
                    &_texture_height);
    }
}

void TabBrowser::_UpdateStatus()
{
    if (_index >= _directory->GetSize())
    {
        _status_text.clear();
        return;
    }

    const DirItem &item = _directory->GetItem(_index);

    _status_text = EnterButton == SCE_CTRL_CIRCLE ? BUTTON_CIRCLE : BUTTON_CROSS;
    _status_text += item.is_dir ? TEXT(BROWSER_ENTER_DIR) : TEXT(BROWSER_START_GAME);
    _status_text += "\t";
    _status_text += EnterButton == SCE_CTRL_CIRCLE ? BUTTON_CROSS : BUTTON_CIRCLE;
    _status_text += TEXT(BROWSER_BACK_DIR);
    _status_text += "\t";
    // _status_text += BUTTON_TRIANGLE;
    // _status_text += TEXT(BROWSER_SEARCH);
    // _status_text += "\t";

    // if (_directory->GetSearchString().size() > 1)
    // {
    //     _status_text += BUTTON_SQUARE;
    //     _status_text += TEXT(BROWSER_NEXT);
    //     _status_text += "\t";
    // }

    if (!item.is_dir)
    {
        _status_text += BUTTON_START;
        if (gFavorites->find(item.name) == gFavorites->end())
        {
            _status_text += TEXT(BROWSER_ADD_FAVORITE);
        }
        else
        {
            _status_text += TEXT(BROWSER_REMOVE_FAVORITE);
        }
    }
}

void TabBrowser::ChangeLanguage(uint32_t language)
{
    LogFunctionName;
    _UpdateStatus();
}

// void TabBrowser::_OnKeyTriangle(Input *input)
// {
//     LogFunctionName;

//     SceImeParam param;
//     param.languagesForced = SCE_FALSE;
//     param.type = SCE_IME_DIALOG_DIALOG_MODE_DEFAULT;
//     param.option = 0;
//     param.filter = NULL;
//     param.inputTextBuffer = (SceWChar16 *)_buf;
//     param.maxTextLength = INPUT_BUF_SIZE;
//     param.initialText = (SceWChar16 *)u"";

//     sceImeParamInit(&param);

//     SceImeDialogParam param;
//     SceImeDialogResult result{0};
//     uint16_t buf[128];

//     // gVideo->Lock();

//     sceImeDialogParamInit(&param);

//     param.inputMethod = 0;
//     param.languagesForced = SCE_FALSE;
//     param.option = 0;
//     param.filter = NULL;
//     param.dialogMode = SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL;
//     param.textBoxMode = SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR;
//     param.supportedLanguages = 0;
//     param.type = SCE_IME_DIALOG_DIALOG_MODE_DEFAULT;
//     param.initialText = (SceWChar16 *)u"";
//     param.inputTextBuffer = (SceWChar16 *)buf;
//     param.title = (const SceWChar16 *)u"Search";
//     param.maxTextLength = 128;
//     param.enterLabel = SCE_IME_ENTER_LABEL_SEARCH;

//     if (sceImeDialogInit(&param) != SCE_OK)
//     {
//         goto END;
//     }

//     while (sceImeDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED)
//     {
//         sceKernelDelayThread(200000);
//     }

//     if (sceImeDialogGetResult(&result) == SCE_OK)
//     {
//         LogDebug("  result:%d button:%d", result.result, result.button);
//         if (result.result == SCE_COMMON_DIALOG_RESULT_OK)
//         {
//             char utf8[512];
//             Utils::Utf16leToUtf8(buf, utf8, 512);

//             size_t count = _directory->Search(utf8);
//             if (count == 0)
//             {
//                 gUi->SetHint(TEXT(TEXT_NOT_FOUND));
//             }
//             else if (count == 1)
//             {

//                 snprintf(utf8, 512, "%s 1 %s", TEXT(TEXT_FOUND), TEXT(TEXT_FILE));
//                 gUi->SetHint(utf8);
//             }
//             else
//             {
//                 snprintf(utf8, 512, "%s %d %s", TEXT(TEXT_FOUND), count, TEXT(TEXT_FILES));
//                 gUi->SetHint(utf8);
//             }

//             _OnKeySquare(input);
//             _UpdateStatus();
//         }
//     }

//     sceImeDialogTerm();

// END:
//     return;
//     // gVideo->Unlock();
// }

// void TabBrowser::_OnKeySquare(Input *input)
// {
//     LogFunctionName;

//     auto results = _directory->GetSearchResults();
//     if (results.size() == 0)
//     {
//         return;
//     }
//     else if (results.size() == 1)
//     {
//         _index = results[0];
//     }
//     else
//     {
//         for (const auto &r : results)
//         {
//             if (r > _index)
//             {
//                 _index = r;
//                 break;
//             }
//         }
//     }
// }