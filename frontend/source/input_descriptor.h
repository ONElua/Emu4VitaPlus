#pragma once
#include <libretro.h>
#include "language_string.h"
#include "defines.h"

class InputDescriptor
{
public:
    InputDescriptor(const char *icon) : _icon(icon), _full(icon), _desc("") {};
    virtual ~InputDescriptor() {}

    const char *Get() { return _full.c_str(); };
    void Update() { _full = _icon + " " + _desc.Get(); };

    void SetDescription(const char *desc)
    {
        _desc = LanguageString(desc);
        Update();
    };

private:
    std::string _icon;
    LanguageString _desc;
    std::string _full;
};

class InputDescriptors
{
public:
    InputDescriptors();
    virtual ~InputDescriptors();
    void UpdateInputDescriptors(const retro_input_descriptor *descriptors);
    const char *Get(int index);
    bool Load(const char *path = APP_INPUT_DESC_PATH);
    bool Save(const char *path = APP_INPUT_DESC_PATH);

private:
    InputDescriptor _descriptors[16];
};

extern InputDescriptors gInputDescriptors;