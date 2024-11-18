#include "touch.h"
#include "log.h"

SceTouchPanelInfo Touch::_info{0};

Touch::Touch(SceTouchPortType port)
    : _port(port),
      _enabled(false),
      _x_scale(1.f),
      _y_scale(1.f)
{
    LogFunctionName;

    if (_info.maxAaX == 0)
    {
        sceTouchGetPanelInfo(_port, &_info);
        LogDebug("%d:\n"
                 "   %d %d %d %d\n"
                 "   %d %d %d %d",
                 _port,
                 _info.minAaX, _info.minAaY, _info.maxAaX, _info.maxAaY,
                 _info.minDispX, _info.minDispY, _info.maxDispX, _info.maxDispY);
    }

    sceKernelCreateLwMutex(&_mutex, "thread_mutex", 0, 0, NULL);
}

Touch::~Touch()
{
    LogFunctionName;
    sceKernelDeleteLwMutex(&_mutex);
}

void Touch::Enable(bool enable)
{
    LogFunctionName;
    _enabled = enable;
    sceTouchSetSamplingState(_port, enable ? SCE_TOUCH_SAMPLING_STATE_START : SCE_TOUCH_SAMPLING_STATE_STOP);
}

void Touch::Poll()
{
    if (!_enabled)
    {
        return;
    }

    SceTouchData touch_data{0};
    if (sceTouchPeek(_port, &touch_data, 1) == 1)
    {

        _last_id = _current_id;
        _current_id = touch_data.report->id;
        _last_axis.x = _axis.x;
        _last_axis.y = _axis.y;
        _axis.x = touch_data.report->x >> 1;
        _axis.y = touch_data.report->y >> 1;
    }
}

void Touch::InitMovingScale(float xscale, float yscale)
{
    LogFunctionName;
    size_t sizex = _info.maxAaX - _info.minAaX;
    size_t sizey = _info.maxAaY - _info.minAaY;
    _Lock();

    _scale_map_table_x.clear();
    _scale_map_table_x.reserve(sizex);
    for (size_t i = 0; i < sizex; i++)
    {
        _scale_map_table_x.emplace_back(i * xscale);
    }

    _scale_map_table_y.clear();
    _scale_map_table_y.reserve(sizey);
    for (size_t i = 0; i < sizey; i++)
    {
        _scale_map_table_y.emplace_back(i * xscale);
    }

    _Unlock();
}