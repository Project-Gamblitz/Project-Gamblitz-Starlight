#pragma once
#include "types.h"
#include <vector>
#include <cstring>
#include <string>
#include <stdarg.h>
#include "nn/fs.h"
#include "nn/os.h"
#include <string>
#include "flexlion/ProcessMemory.hpp"
#include "sead/stack.h"
#include "flexlion/Thread.hpp"
#include <queue>

class FsLogger
{
    public:
    struct QueueEntry{
        void *data;
        size_t length;
        char *sdpath;
        s64 offset;
    };
    static u32 Initialize();
    static void threadMain();
    static void addToQueue(const char *sdpath, s64 offset, void* data, size_t length);
    static u32 InitializeFile(const char *sdpath);
    static u32 LogDataToSd(const char *sdpath, s64 offset, void* data, size_t length, nn::fs::WriteOptionFlag flag);
    static u32 Log(const char *sdpath, s64 offset, const char* data);
    static u32 LogFormat(const char *sdpath, s64 offset, const char* data, ...);
    static u32 LogFormatDirect(const char *sdpath, s64 offset, const char* data, ...);
    static u32 LogDefault(const char* data);
    static u32 LogDefaultDirect(const char* data);
    static u32 LogFormatDefault(const char* data, ...);
    static u32 LogFormatDefaultDirect(const char* data, ...);
};