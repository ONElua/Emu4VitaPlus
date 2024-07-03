#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <string.h>
#include "file.h"
#include "log.h"

#define SCE_STM_RWU (00600)
#define SCE_STM_RU (00400)

namespace File
{
    bool Exist(const char *name)
    {
        SceIoStat stat;
        return sceIoGetstat(name, &stat) == SCE_OK;
    }

    size_t GetSize(const char *name)
    {
        SceIoStat stat;
        if (sceIoGetstat(name, &stat) == SCE_OK)
        {
            return stat.st_size;
        }

        return 0;
    }

    bool ReadFile(const char *name, void *buf, SceSSize size)
    {
        SceUID fd = sceIoOpen(name, SCE_O_RDONLY, SCE_STM_RU);
        if (fd <= 0)
        {
            return false;
        }

        SceSSize s = sceIoRead(fd, buf, size);
        sceIoClose(fd);

        return s == size;
    }

    bool GetCreateTime(const char *name, SceDateTime *time)
    {
        SceIoStat stat;
        if (sceIoGetstat(name, &stat) != SCE_OK)
        {
            LogWarn("failed to get stat: %s", name);
            return false;
        }

        SceRtcTick tick;
        sceRtcGetTick(&stat.st_mtime, &tick);
        sceRtcConvertUtcToLocalTime(&tick, &tick);
        sceRtcSetTick(time, &tick);

        return true;
    }

    bool GetCreateTime(const char *name, time_t *time)
    {
        SceDateTime t;
        if (GetCreateTime(name, &t))
        {
            sceRtcGetTime_t(&t, time);
            return true;
        }
        return false;
    }

    void MakeDirs(const char *path, SceIoMode mode)
    {
        if (!(path && *path) || Exist(path))
        {
            return;
        }

        char *_path = new char[strlen(path) + 1];
        strcpy(_path, path);
        char *p = _path;
        do
        {
            p = strchr(p, '/');
            if (p)
            {
                *p = '\0';
            }

            if (!Exist(_path))
            {
                sceIoMkdir(_path, mode);
            }

            if (p)
            {
                *p = '/';
                p++;
            }
            else
            {
                break;
            }
        } while (true);

        delete[] _path;
    }

    bool Remove(const char *path)
    {
        return sceIoRemove(path) == SCE_OK;
    }

    static const char *_GetDirEndPos(const char *path)
    {
        char *p = strrchr(path, '/');
        if (p)
            return p;
        return strrchr(path, ':');
    }

    static char *_GetDotPos(const char *path)
    {
        return strrchr(path, '.');
    }

    std::string GetName(const char *path)
    {
        const char *start = _GetDirEndPos(path);
        if (start)
        {
            start++;
            return start;
        }
        else
        {
            return path;
        }
    }

    std::string GetStem(const char *path)
    {
        const char *start = _GetDirEndPos(path);
        const char *end = _GetDotPos(path);

        if (!start)
            start = path;
        else
            start++;

        if (!end)
            end = path + strlen(path);

        return std::string(start, end - start);
    }

    std::string GetDir(const char *path)
    {
        const char *end = _GetDirEndPos(path);
        if (end)
        {
            if (*(end - 1) == ':')
                end--;
            return std::string(path, end - path + 1);
        }
        else
        {
            return path;
        }
    }
}
