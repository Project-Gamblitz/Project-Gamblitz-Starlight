#pragma once

#include "types.h"
#include "nn/fs.h"
#include "mem.h"
#include "memory.h"
#include "malloc.h"
#include "str.h"
#include "string.h"
#include "string"
#include "SCrypto.hpp"

class FileReader{
    public:
    class File{
        public:
        File(){
            data = NULL;
            size = 0;
            buffer = NULL;
        };
        ~File(){
            if(data != NULL and buffer == NULL){
                free(data);
            }
            data = NULL;
            size = NULL;
            buffer = NULL;
        };
        bool hasContent(){
            return data != NULL and size != NULL;
        };
        void *data;
        void *buffer;
        u64 size;
    };
    static Result readFileFromSd(const char *fileName, FileReader::File *file);
};