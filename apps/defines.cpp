#include <stdint.h>
#include <libretro.h>
#include <vector>
#include <psp2common/ctrl.h>
#include "config_types.h"
#include "input.h"
#include "config.h"

#ifndef ROOT_DIR
#define ROOT_DIR "ux0:data/EMU4VITAPLUS"
#endif

#define _APP_DATA_DIR ROOT_DIR "/" _APP_DIR_NAME

const char APP_DATA_DIR[] = _APP_DATA_DIR;
const char APP_LOG_PATH[] = _APP_DATA_DIR "/Emu4Vita++.log";
const char APP_CONFIG_PATH[] = _APP_DATA_DIR "/config.ini";
const char APP_CORE_CONFIG_PATH[] = _APP_DATA_DIR "/core.ini";
const char CORE_SYSTEM_DIR[] = _APP_DATA_DIR "/system";
const char CORE_SAVEFILES_DIR[] = _APP_DATA_DIR "/savefiles";
const char CORE_SAVESTATES_DIR[] = _APP_DATA_DIR "/savestates";
const char APP_TITLE_NAME[] = _APP_TITLE_NAME;
const char CORE_FULL_NAME[] = _CORE_FULL_NAME;
const char APP_VER_STR[] = _APP_VER_STR;
const char CONSOLE[] = _CONSOLE;
const char CONSOLE_DIR[] = "app0:data/" _CONSOLE;

const uint8_t RETRO_KEYS[] = {
    RETRO_DEVICE_ID_NONE,
    RETRO_DEVICE_ID_JOYPAD_UP,
    RETRO_DEVICE_ID_JOYPAD_DOWN,
    RETRO_DEVICE_ID_JOYPAD_LEFT,
    RETRO_DEVICE_ID_JOYPAD_RIGHT,
#if defined(GBA_BUILD)
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_L,
    RETRO_DEVICE_ID_JOYPAD_R,
#elif defined(ARC_BUILD)
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_X,
    RETRO_DEVICE_ID_JOYPAD_Y,
    RETRO_DEVICE_ID_JOYPAD_L,
    RETRO_DEVICE_ID_JOYPAD_R,
    RETRO_DEVICE_ID_JOYPAD_L2,
    RETRO_DEVICE_ID_JOYPAD_R2,
#elif defined(SFC_BUILD)
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_X,
    RETRO_DEVICE_ID_JOYPAD_Y,
    RETRO_DEVICE_ID_JOYPAD_L,
    RETRO_DEVICE_ID_JOYPAD_R,
#elif defined(FC_BUILD) || defined(GBC_BUILD) || defined(NGP_BUILD) || defined(WSC_BUILD)
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
#elif defined(MD_BUILD) || defined(PCE_BUILD)
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_X,
    RETRO_DEVICE_ID_JOYPAD_Y,
    RETRO_DEVICE_ID_JOYPAD_L,
    RETRO_DEVICE_ID_JOYPAD_R,
#elif defined(PS_BUILD)
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_X,
    RETRO_DEVICE_ID_JOYPAD_Y,
    RETRO_DEVICE_ID_JOYPAD_L,
    RETRO_DEVICE_ID_JOYPAD_R,
    RETRO_DEVICE_ID_JOYPAD_L2,
    RETRO_DEVICE_ID_JOYPAD_R2,
    RETRO_DEVICE_ID_JOYPAD_L3,
    RETRO_DEVICE_ID_JOYPAD_R3,
#else
#error "unknown build"
#endif
    RETRO_DEVICE_ID_JOYPAD_START,
    RETRO_DEVICE_ID_JOYPAD_SELECT,
};

const size_t RETRO_KEYS_SIZE = (sizeof(RETRO_KEYS) / sizeof(RETRO_KEYS[0]));

const std::vector<ControlMapConfig> CONTROL_MAPS = {
    {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
    {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
    {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
    {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
#if defined(GBA_BUILD)
    {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
    {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_B, true},
    {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
    {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_A, true},
    {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
    {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
    {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
    {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
    {SCE_CTRL_L2},
    {SCE_CTRL_R2},
    {SCE_CTRL_L3},
    {SCE_CTRL_R3},
#elif defined(ARC_BUILD)
    {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
    {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
    {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
    {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
    {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
    {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
    {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
    {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
    {SCE_CTRL_L2, RETRO_DEVICE_ID_JOYPAD_L2},
    {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
    {SCE_CTRL_L3},
    {SCE_CTRL_R3},
#elif defined(SFC_BUILD)
    {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
    {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
    {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
    {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
    {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
    {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
    {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
    {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
    {SCE_CTRL_L2},
    {SCE_CTRL_R2},
    {SCE_CTRL_L3},
    {SCE_CTRL_R3},
#elif defined(FC_BUILD) || defined(GBC_BUILD) || defined(NGP_BUILD) || defined(WSC_BUILD)
    {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
    {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_A, true},
    {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
    {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_B, true},
    {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
    {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
    {SCE_CTRL_L1},
    {SCE_CTRL_R1},
    {SCE_CTRL_L2},
    {SCE_CTRL_R2},
    {SCE_CTRL_L3},
    {SCE_CTRL_R3},
#elif defined(MD_BUILD) || defined(PCE_BUILD)
    {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
    {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
    {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
    {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
    {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
    {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
    {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
    {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
    {SCE_CTRL_L2},
    {SCE_CTRL_R2},
    {SCE_CTRL_L3},
    {SCE_CTRL_R3},
#elif defined(PS_BUILD)
    {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
    {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
    {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
    {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
    {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
    {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
    {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
    {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
    {SCE_CTRL_L2, RETRO_DEVICE_ID_JOYPAD_L2},
    {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
    {SCE_CTRL_L3, RETRO_DEVICE_ID_JOYPAD_L3},
    {SCE_CTRL_R3, RETRO_DEVICE_ID_JOYPAD_R3},
#else
#error "unknown build"
#endif
    {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
    {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
    {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
    {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
    {SCE_CTRL_RSTICK_UP},
    {SCE_CTRL_RSTICK_DOWN},
    {SCE_CTRL_RSTICK_LEFT},
    {SCE_CTRL_RSTICK_RIGHT},
};