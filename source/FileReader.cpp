#include "flexlion/FileReader.hpp"
#include "flexlion/Utils.hpp"

const char *logDir = "sdcard:";
//static char *sdcardname = NULL;
static bool init = 0;

Result FileReader::readFileFromSd(const char *fileName, FileReader::File *file){
    if (file == NULL) return -1;
    //if(!nn::fs::IsSdCardInserted()){
    //    return -1;
    //}
    char str[256];
    strncpy(str, logDir, sizeof(str));
    strncat(str, fileName, sizeof(str));
    nn::fs::MountSdCard("sdcard");
    nn::fs::FileHandle handle;
    R_TRY(nn::fs::OpenFile(&handle, str, nn::fs::OpenMode_Read));

    s64 size;
    Result r = nn::fs::GetFileSize(&size, handle);
    if (R_FAILED(r)) {
        nn::fs::CloseFile(handle);
        return r;
    }

    file->size = size;
    void* bin = file->buffer;
    if(bin == NULL){
        bin = malloc(size + 1);
        if(bin == NULL)
            return -1;
    }
    memset(bin, 0, size + 1);
    u64 tmp;
    //(void (*)())(ProcessMemory::SdkAddr(0x4D434);
    r = nn::fs::ReadFile(&tmp, handle, 0, bin, size);
    if (R_FAILED(r)) {
        if(file->buffer == NULL){
            free(bin);
        }
    } else{
        file->data = bin;
    }
    nn::fs::CloseFile(handle);
    //nn::fs::Unmount("sdcard");
    return r;
}