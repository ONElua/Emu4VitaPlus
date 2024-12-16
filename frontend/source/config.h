#pragma once
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "libretro.h"
#include "core_options.h"
#include "config_types.h"
#include "language_define.h"
#include "defines.h"
#include "input_descriptor.h"
#include "device_options.h"

#define MIN_REWIND_BUF_SIZE 5
#define MAX_REWIND_BUF_SIZE 50
#define REWIND_BUF_SIZE_STEP 5
#define DEFAULT_MUTE false

namespace Emu4Vita
{
    class Config
    {
    public:
        Config();
        virtual ~Config();

        bool Load(const char *path = APP_CONFIG_PATH);
        bool Save(const char *path = APP_CONFIG_PATH);

        void Default();
        void DefaultControlMap();
        void DefaultHotKey();
        void DefaultGraphics();
        retro_language GetRetroLanguage();
        bool FrontEnabled();
        bool RearEnabled();

        std::string version;
        bool boot_from_arch;
        bool reboot_when_loading_again;
        std::string last_rom;
        uint32_t language;
        uint32_t rewind;
        uint32_t rewind_buf_size;
        std::vector<ControlMapConfig> control_maps;
        uint32_t graphics[GRAPHICS_CONFIG_COUNT];
        uint32_t hotkeys[HOT_KEY_COUNT];
        CoreOptions core_options;
        InputDescriptors input_descriptors;
        DeviceOptions device_options;
        uint32_t mute;
        uint32_t auto_save;
        uint32_t mouse;
        uint32_t lightgun;
        uint32_t swap_enter;
        uint32_t sim_button;
        uint32_t independent_core_config;

        static const std::unordered_map<uint32_t, const char *> PsvKeyStr;
        static const std::unordered_map<uint32_t, const char *> HotkeyStr;
        static const std::unordered_map<uint32_t, LanguageString> ControlTextMap;
        static const std::unordered_map<uint32_t, const char *> GraphicsStr;
        static const std::unordered_map<uint8_t, LanguageString> RetroTextMap;

    private:
    };
}

extern Emu4Vita::Config *gConfig;