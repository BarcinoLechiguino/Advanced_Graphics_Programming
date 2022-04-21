#include <stdio.h>
#include <stdlib.h>

#include "windows_includes.h"
#include "globals.h"

#include "file_manager.h"

#define GLOBAL_FRAME_ARENA_SIZE MB(16)
u8* GlobalFrameArenaMemory = nullptr;
u32 GlobalFrameArenaHead = 0;

void FileManager::Init()
{
    GlobalFrameArenaMemory = (u8*)malloc(GLOBAL_FRAME_ARENA_SIZE);
}

void FileManager::CleanUp()
{
    free(GlobalFrameArenaMemory);
}

void FileManager::ResetFrameAllocator()
{
    GlobalFrameArenaHead = 0;
}

String FileManager::MakeString(const char* cstr)
{
    String str = {};
    str.len = Utils::Strlen(cstr);
    str.str = (char*)Utils::PushBytes(cstr, str.len);
    Utils::PushChar(0);
    return str;
}

String FileManager::MakePath(String dir, String filename)
{
    String str = {};
    str.len = dir.len + filename.len + 1;
    str.str = (char*)Utils::PushBytes(dir.str, dir.len);
    Utils::PushChar('/');
    Utils::PushBytes(filename.str, filename.len);
    Utils::PushChar(0);
    return str;
}

String FileManager::GetDirectoryPart(String path)
{
    String str = {};
    i32 len = (i32)path.len;
    while (len >= 0) {
        len--;
        if (path.str[len] == '/' || path.str[len] == '\\')
            break;
    }
    str.len = (u32)len;
    str.str = (char*)Utils::PushBytes(path.str, str.len);
    Utils::PushChar(0);
    return str;
}

// Reads a whole file and returns a string with its contents. 
// The returned string is temporary and should be copied if it needs to persist for several frames.
String FileManager::ReadTextFile(const char* filepath)
{
    String fileText = {};

    FILE* file = fopen(filepath, "rb");

    if (file)
    {
        fseek(file, 0, SEEK_END);
        fileText.len = ftell(file);
        fseek(file, 0, SEEK_SET);

        fileText.str = (char*)Utils::PushSize(fileText.len + 1);
        fread(fileText.str, sizeof(char), fileText.len, file);
        fileText.str[fileText.len] = '\0';

        fclose(file);
    }
    else
    {
        ELOG("fopen() failed reading file %s", filepath);
    }

    return fileText;
}

// It retrieves a timestamp indicating the last time the file was modified. 
// Can be useful in order to check for file modifications to implement hot reloads.
u64 FileManager::GetFileLastWriteTimestamp(const char* filepath)
{
    #ifdef _WIN32
        union Filetime2u64
        {
            FILETIME filetime;
            u64      u64time;
        } conversor;

        WIN32_FILE_ATTRIBUTE_DATA Data;
        if (GetFileAttributesExA(filepath, GetFileExInfoStandard, &Data))
        {
            conversor.filetime = Data.ftLastWriteTime;
            return(conversor.u64time);
        }
    #else
        // NOTE: This has not been tested in unix-like systems
        struct stat attrib;
        return (stat(filepath, &attrib) == 0) ? attrib.st_mtime : 0;
    #endif

    return 0;
}

// UTILS -------------------------------------------------------------------
u32 FileManager::Utils::Strlen(const char* string)
{
    u32 len = 0;
    while (*string++) len++;
    return len;
}

void* FileManager::Utils::PushSize(u32 byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE,
        "Trying to allocate more temp memory than available");

    u8* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead += byteCount;
    return curPtr;
}

void* FileManager::Utils::PushBytes(const void* bytes, u32 byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE,
        "Trying to allocate more temp memory than available");

    u8* srcPtr = (u8*)bytes;
    u8* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    u8* dstPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead += byteCount;
    while (byteCount--) *dstPtr++ = *srcPtr++;
    return curPtr;
}

u8* FileManager::Utils::PushChar(u8 c)
{
    ASSERT(GlobalFrameArenaHead + 1 <= GLOBAL_FRAME_ARENA_SIZE,
        "Trying to allocate more temp memory than available");
    u8* ptr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead++;
    *ptr = c;
    return ptr;
}