#pragma once
#include <string>
#include <libretro.h>
#include <stdint.h>
#include "texture_buf.h"
#include "audio.h"
#include "delay.h"
#include "input.h"

struct Rect
{
    float x = 0.f;
    float y = 0.f;
    float width = 0.f;
    float height = 0.f;
};

class Emulator
{
public:
    Emulator();
    virtual ~Emulator();

    void Init();
    bool LoadGame(const char *path);
    void UnloadGame();
    void Reset();
    void Run();
    void Show();
    void Lock();
    void Unlock();
    void SetSpeed(double speed);
    const char *CurrentName() { return _current_name.c_str(); };
    bool SaveScreenShot(const char *name, size_t height = 0);
    bool GetCurrentSoftwareFramebuffer(retro_framebuffer *fb);
    const char *GetValidExtensions() const { return _info.valid_extensions; };
    double GetSampleRate() const { return _av_info.timing.sample_rate; };
    float GetAspectRatio() { return _av_info.geometry.aspect_ratio; };

    friend bool EnvironmentCallback(unsigned cmd, void *data);
    friend void VideoRefreshCallback(const void *data, unsigned width, unsigned height, size_t pitch);
    friend size_t AudioSampleBatchCallback(const int16_t *data, size_t frames);
    friend void InputPollCallback();
    friend int16_t InputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id);

private:
    void _SetPixelFormat(retro_pixel_format format);
    void _SetVideoSize(uint32_t width, uint32_t height);
    void _SetupKeys();
    void _OnPsButton(Input *input);

    std::string _current_name;
    retro_system_info _info;
    retro_system_av_info _av_info;

    SceGxmTextureFormat _video_pixel_format;
    retro_pixel_format _retro_pixel_format;
    TextureBuf *_texture_buf;
    Rect _video_rect;

    Audio _audio;
    Input _input;
    uint32_t _keys[16];
    uint32_t _keys_mask;
    double _speed;
    Delay _delay;
    bool _soft_frame_buf_render;
    vita2d_texture *_current_tex;

    SceKernelLwMutexWork _tex_mutex;
    SceKernelLwCondWork _tex_cond;
    SceKernelLwMutexWork _run_mutex;
};

extern Emulator *gEmulator;