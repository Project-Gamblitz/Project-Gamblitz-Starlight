#include "flexlion/FsLogger.hpp"
#include "flexlion/Utils.hpp"

static int curLogOffset = 0;
static Flexlion::Thread *fsloggerthread;
static char *defLogFile = "/gamblitz-log.txt";
const char *defLogDir = "sdcard:";
//static char *sdcardname;
//char *exceptionhandler = 
std::queue<FsLogger::QueueEntry*> *fsloggerqueue;

u32 FsLogger::Initialize(){
    nn::fs::MountSdCard("sdcard");
    nn::fs::DirectoryEntryType entryType;
    Result rc = nn::fs::GetEntryType(&entryType, "sdcard:/gamblitz-log.txt");
    if (rc != 0x202) {
        R_TRY(nn::fs::DeleteFile("sdcard:/gamblitz-log.txt"));
    }
    FsLogger::LogDefaultDirect("");
    //}
    fsloggerthread = new Flexlion::Thread((voidFunc)(&FsLogger::threadMain), Flexlion::ThreadType::cDefault, 10);
    fsloggerthread->start();
}

void FsLogger::threadMain(){
    if (fsloggerqueue == NULL){
        return;
    }
    while (!fsloggerqueue->empty()) {
        FsLogger::QueueEntry *entry = fsloggerqueue->front();
        FsLogger::LogDataToSd(entry->sdpath, entry->offset, entry->data, entry->length, nn::fs::WriteOptionFlag::WriteOptionFlag_Flush);
        fsloggerqueue->pop();
        delete []entry->data;
        delete []entry;
    }
}

u32 FsLogger::InitializeFile(const char *sdpath){
    char str[256];
    //if(nn::fs::IsSdCardInserted()){
    nn::fs::MountSdCard("sdcard");
    strncpy(str, defLogDir, sizeof(str));
    strncat(str, sdpath, sizeof(str));
    nn::fs::DirectoryEntryType entryType;
    Result rc = nn::fs::GetEntryType(&entryType, str);
    if (rc != 0x202) {
        R_TRY(nn::fs::DeleteFile(str));
    }
    FsLogger::LogDataToSd(sdpath, 0, (void*)"", strlen(""), nn::fs::WriteOptionFlag::WriteOptionFlag_Flush);
    //}
}

u32 FsLogger::LogDataToSd(const char *sdpath, s64 offset, void* data, size_t length, nn::fs::WriteOptionFlag flag){
    char str[256];
    //if(nn::fs::IsSdCardInserted()){
    nn::fs::MountSdCard("sdcard");
    strncpy(str, defLogDir, sizeof(str));
    strncat(str, sdpath, sizeof(str));
    //str = t1.c_str();
    nn::fs::DirectoryEntryType entryType;
    Result rc = nn::fs::GetEntryType(&entryType, str);
    if (rc == 0x202) {  // Path does not exist
        R_TRY(nn::fs::CreateFile(str, offset + length));
    } else if (R_FAILED(rc))
        return rc;
    if (entryType == nn::fs::DirectoryEntryType_Directory) return -1;
    nn::fs::FileHandle handle;
    R_TRY(nn::fs::OpenFile(&handle, str, nn::fs::OpenMode_ReadWrite | nn::fs::OpenMode_Append));
    Result r;
    s64 fileSize;
    r = nn::fs::GetFileSize(&fileSize, handle);
    if (R_FAILED(r)) {
        nn::fs::CloseFile(handle);
        return r;
    }
    if (fileSize < offset + (s64)length) {  // make sure we have enough space
        r = nn::fs::SetFileSize(handle, offset + length);

        if (R_FAILED(r)) {
            nn::fs::CloseFile(handle);
            return r;
        }
    }
    r = nn::fs::WriteFile(handle, offset, data, length, nn::fs::WriteOption::CreateOption(flag));
    nn::fs::CloseFile(handle);
    //nn::fs::Unmount("sdcard");
    return r;
    //}
}
u32 FsLogger::Log(const char *sdpath, s64 offset, const char* data){
    FsLogger::addToQueue(sdpath, offset, (void*)data, strlen(data));
}

void FsLogger::addToQueue(const char *sdpath, s64 offset, void* data, size_t length){
    if(fsloggerqueue == NULL){
        fsloggerqueue = new std::queue<FsLogger::QueueEntry*>();
        if(fsloggerqueue == NULL){
            return;
        }
    }
    FsLogger::QueueEntry *entry = new FsLogger::QueueEntry;
    entry->data = new char[length + 2];
    memset(entry->data, 0, length + 2);
    memcpy(entry->data, data, length);
    entry->sdpath = new char[strlen(sdpath) + 2];
    memset(entry->sdpath, 0, strlen(sdpath) + 2);
    memcpy(entry->sdpath, sdpath, strlen(sdpath));
    entry->length = length;
    entry->offset = offset;
    fsloggerqueue->push(entry);
}

u32 FsLogger::LogFormat(const char *sdpath, s64 offset, const char* data, ...){
    //if(nn::fs::IsSdCardInserted()){
    va_list args;
    char buff[0x1000];
    memset(buff, 0, sizeof(buff));
    va_start(args, data);
    int len = vsnprintf(buff, sizeof(buff), data, args);
    FsLogger::addToQueue(sdpath, offset, buff, len);
    va_end (args);
    return len;
    //};
    return 0;
}

u32 FsLogger::LogFormatDirect(const char *sdpath, s64 offset, const char* data, ...){
    //if(nn::fs::IsSdCardInserted()){
    va_list args;
    char buff[0x1000];
    memset(buff, 0, sizeof(buff));
    va_start(args, data);
    int len = vsnprintf(buff, sizeof(buff), data, args);
    FsLogger::LogDataToSd(sdpath, offset, buff, len, nn::fs::WriteOptionFlag::WriteOptionFlag_Flush);
    va_end (args);
    return len;
    //};
    return 0;
}

u32 FsLogger::LogDefault(const char* data){
    //if(nn::fs::IsSdCardInserted()){
    FsLogger::Log(defLogFile, curLogOffset, data);
    curLogOffset+=strlen(data);
    //}
}

u32 FsLogger::LogDefaultDirect(const char* data){
    //if(nn::fs::IsSdCardInserted()){
    FsLogger::LogDataToSd(defLogFile, curLogOffset, (void*)data, strlen(data), nn::fs::WriteOptionFlag::WriteOptionFlag_Flush);
    curLogOffset+=strlen(data);
    //}
}

u32 FsLogger::LogFormatDefault(const char* data, ...){
    //if(nn::fs::IsSdCardInserted()){
    va_list args;
    char buff[0x1000];
    memset(buff, 0, sizeof(buff));
    va_start(args, data);
    int len = vsnprintf(buff, sizeof(buff), data, args);
    FsLogger::addToQueue(defLogFile, curLogOffset, buff, len);
    va_end (args);
    curLogOffset+=len;
    //};
}

u32 FsLogger::LogFormatDefaultDirect(const char* data, ...){
    //if(nn::fs::IsSdCardInserted()){
    va_list args;
    char buff[0x1000];
    memset(buff, 0, sizeof(buff));
    va_start(args, data);
    int len = vsnprintf(buff, sizeof(buff), data, args);
    FsLogger::LogDataToSd(defLogFile, curLogOffset, buff, len, nn::fs::WriteOptionFlag::WriteOptionFlag_Flush);
    va_end (args);
    curLogOffset+=len;
    //};
}