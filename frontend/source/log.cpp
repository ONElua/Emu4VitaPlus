#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <psp2common/kernel/threadmgr.h>
#include "file.h"
#include "log.h"

#if LOG_LEVEL != LOG_LEVEL_OFF
Log *gLog = NULL;
#endif

Log::Log(const char *name, int buf_len)
{
	File::Remove(name);
	_name = name;
	_buf = new char[buf_len];
	_buf_len = buf_len;
	sceKernelCreateLwMutex(&_mutex, "log_mutex", 0, 0, NULL);
}

Log::~Log()
{
	delete[] _buf;
	sceKernelDeleteLwMutex(&_mutex);
}

void Log::log(int log_level, const char *format, ...)
{
	sceKernelLockLwMutex(&_mutex, 1, NULL);
	FILE *fp = fopen(_name.c_str(), "a");
	if (fp)
	{
		va_list args;
		va_start(args, format);
		vsnprintf(_buf, _buf_len, format, args);
		va_end(args);

		SceDateTime time;
		sceRtcGetCurrentClockLocalTime(&time);
		fprintf(fp, "[%c] %02d:%02d:%02d.%03d %s\n", LogLevelChars[log_level], time.hour, time.minute, time.second, time.microsecond / 1000, _buf);
		fclose(fp);
	}
	sceKernelUnlockLwMutex(&_mutex, 1);
}
