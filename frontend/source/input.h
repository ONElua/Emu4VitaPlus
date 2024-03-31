#pragma once
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include "delay.h"

typedef std::function<void()> InputFunc;
const uint64_t SCE_CTRL_LSTICK_UP = 0x0000000100000000;
const uint64_t SCE_CTRL_LSTICK_RIGHT = 0x0000000200000000;
const uint64_t SCE_CTRL_LSTICK_DOWN = 0x0000000400000000;
const uint64_t SCE_CTRL_LSTICK_LEFT = 0x0000000800000000;
const uint64_t SCE_CTRL_RSTICK_UP = 0x0000001000000000;
const uint64_t SCE_CTRL_RSTICK_RIGHT = 0x0000002000000000;
const uint64_t SCE_CTRL_RSTICK_DOWN = 0x0000004000000000;
const uint64_t SCE_CTRL_RSTICK_LEFT = 0x0000008000000000;

#define DEFAULT_TURBO_INTERVAL 300000ull

class Input
{
public:
    Input();
    virtual ~Input();

    void SetKeyUpCallback(uint64_t key, InputFunc func);
    void SetKeyDownCallback(uint64_t key, InputFunc func);

    void UnsetKeyUpCallback(uint64_t key);
    void UnsetKeyDownCallback(uint64_t key);

    void SetKeyTurbo(uint64_t key);
    void UnsetKeyTurbo(uint64_t key);
    void SetTurboInterval(uint64_t turbo_interval);

    void Poll();

private:
    std::unordered_map<uint64_t, InputFunc> _key_up_callbacks;
    std::unordered_map<uint64_t, InputFunc> _key_down_callbacks;
    uint64_t _last_key;
    uint64_t _turbo_key;
    uint64_t _last_active;
    Delay *_delay;
};