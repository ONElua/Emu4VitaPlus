#include "emulator.h"
#include "app.h"
#include "video.h"
#include "state_manager.h"
#include "config.h"

void Emulator::_SetupKeys()
{
    LogFunctionName;
    memset(_keys, 0, sizeof(_keys));
    _keys_mask = 0;
    for (const auto &k : gConfig->control_maps)
    {
        if (k.retro == 0xff)
        {
            continue;
        }

        if (k.retro >= 16)
        {
            LogError("  wrong key config: %d %08x", k.retro, k.psv);
            continue;
        }
        _keys[k.retro] |= k.psv;
        _keys_mask |= k.psv;
        if (k.turbo)
        {
            _input.SetTurbo(k.psv);
        }
    }

    // for (int i = 0; i < 16; i++)
    // {
    //     LogDebug("%d %08x", i, _keys[i]);
    // }

#define BIND_HOTKEY(KEY, FUNC) _input.SetKeyDownCallback(gConfig->hotkeys[KEY], std::bind(&Emulator::FUNC, this, &_input));
#define BIND_HOTKEY_UP(KEY, FUNC) _input.SetKeyUpCallback(gConfig->hotkeys[KEY], std::bind(&Emulator::FUNC, this, &_input));

    BIND_HOTKEY(SAVE_STATE, _OnHotkeySave);
    BIND_HOTKEY(LOAD_STATE, _OnHotkeyLoad);
    BIND_HOTKEY(GAME_SPEED_UP, _OnHotkeySpeedUp);
    BIND_HOTKEY(GAME_SPEED_DOWN, _OnHotkeySpeedDown);
    BIND_HOTKEY(GAME_REWIND, _OnHotkeyRewind);
    BIND_HOTKEY(CONTROLLER_PORT_UP, _OnHotkeyCtrlPortUp);
    BIND_HOTKEY(CONTROLLER_PORT_DOWN, _OnHotkeyCtrlPortDown);
    BIND_HOTKEY(EXIT_GAME, _OnHotkeyExitGame);

    BIND_HOTKEY_UP(GAME_REWIND, _OnHotkeyRewindUp);
    _input.SetKeyUpCallback(SCE_CTRL_PSBUTTON, std::bind(&Emulator::_OnPsButton, this, &_input));
}

void Emulator::_OnPsButton(Input *input)
{
    LogFunctionName;
    gVideo->Lock();
    gStatus = APP_STATUS_SHOW_UI_IN_GAME;
    gVideo->Unlock();
    Save();
    LogDebug("_OnPsButton end");
}

void Emulator::_OnHotkeySave(Input *input)
{
    LogFunctionName;
    Lock();
    gStateManager->states[0]->Save();
    Unlock();
}

void Emulator::_OnHotkeyLoad(Input *input)
{
    LogFunctionName;
    Lock();
    gStateManager->states[0]->Load();
    Unlock();
}

void Emulator::_OnHotkeySpeedUp(Input *input)
{
    LogFunctionName;
    if (_speed < 2.0f)
    {
        SetSpeed(_speed + 0.1);
    }
}

void Emulator::_OnHotkeySpeedDown(Input *input)
{
    LogFunctionName;
    if (_speed > 0.3)
    {
        SetSpeed(_speed - 0.1);
    }
}

void Emulator::_OnHotkeyRewind(Input *input)
{
    _rewind_manager.StartRewind();
}

void Emulator::_OnHotkeyRewindUp(Input *input)
{
    _rewind_manager.StopRewind();
}

void Emulator::_OnHotkeyCtrlPortUp(Input *input)
{
    LogFunctionName;
}

void Emulator::_OnHotkeyCtrlPortDown(Input *input)
{
    LogFunctionName;
}

void Emulator::_OnHotkeyExitGame(Input *input)
{
    LogFunctionName;
    gVideo->Lock();
    UnloadGame();
    gVideo->Unlock();
}