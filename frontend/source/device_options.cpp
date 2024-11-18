#include "device_options.h"
#include "log.h"

#define DEVCIE_SECTION "DEVICE"
static const char *Unset = gTexts[LANGUAGE_ENGLISH][UNSET];

const std::vector<LanguageString> ControllerTypes::GetValues() const
{
    std::vector<LanguageString> _values;
    _values.reserve(this->size() + 1);
    _values.emplace_back(TEXT(UNSET));
    for (const auto &v : *this)
    {
        _values.emplace_back(v.desc);
    }

    return _values;
}

size_t ControllerTypes::GetValueIndex()
{
    for (size_t i = 0; i < this->size(); i++)
    {
        if (value == (*this)[i].desc)
        {
            return i + 1;
        }
    }

    return 0;
}

void ControllerTypes::SetValueIndex(size_t index)
{
    if (index == 0 || index > this->size())
    {
        value = Unset;
    }
    else
    {
        value = (*this)[index - 1].desc;
    }

    LogDebug("SetValueIndex %d %s", index, value.c_str());
}

void ControllerTypes::Default()
{
    value = Unset;
}

void ControllerTypes::Apply(uint32_t port) const
{
    LogFunctionName;

    for (const auto &v : *this)
    {
        if (value == v.desc)
        {
            LogDebug("  retro_set_controller_port_device: %d %x", port, v.device);
            retro_set_controller_port_device(port, v.device);
            break;
        }
    }
}

void DeviceOptions::Load(retro_controller_info *info)
{
    LogFunctionName;
    unsigned count = 0;
    while (info->types)
    {
        ControllerTypes *types;
        if (count >= this->size())
        {
            this->push_back(ControllerTypes{});
        }
        types = &(*this)[count];
        types->clear();

        bool found = false;
        for (unsigned i = 0; i < info->num_types; i++)
        {
            LogDebug(" %d %08x %s", count, info->types[i].id, info->types[i].desc);
            types->push_back(ControllerType{info->types[i].desc, info->types[i].id});
            if (types->value == info->types[i].desc)
            {
                found = true;
            }
        }
        if (!found)
            types->value = Unset;

        info++;
        count++;
    }
}

bool DeviceOptions::Load(CSimpleIniA &ini)
{
    LogFunctionName;
    char tmp[32];
    int num_types = ini.GetLongValue(DEVCIE_SECTION, "num_types");
    if (num_types == 0)
        return false;

    this->clear();
    this->reserve(num_types);
    for (int i = 0; i < num_types; i++)
    {
        snprintf(tmp, 32, "device_%02d", i);
        ControllerTypes types;
        types.value = ini.GetValue(DEVCIE_SECTION, tmp, Unset);
        this->emplace_back(types);
    }

    return true;
}

bool DeviceOptions::Save(CSimpleIniA &ini)
{
    char tmp[32];
    int count = 0;
    for (auto const &iter : *this)
    {
        if (iter.value != Unset)
        {
            snprintf(tmp, 32, "device_%02d", count);
            ini.SetValue(DEVCIE_SECTION, tmp, iter.value.c_str());
        }
        count++;
    }
    ini.SetLongValue(DEVCIE_SECTION, "num_types", count);

    return true;
}