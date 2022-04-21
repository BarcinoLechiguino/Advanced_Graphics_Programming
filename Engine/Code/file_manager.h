#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "base_types.h"

namespace FileManager
{
    void    Init();
    void    CleanUp();
    
    void    ResetFrameAllocator();

    String  MakeString(const char* cstr);
    String  MakePath(String dir, String filename);
    String  GetDirectoryPart(String path);

    String  ReadTextFile(const char* filepath);                     // Reads a whole file and returns a string with its contents.
    u64     GetFileLastWriteTimestamp(const char* filepath);        // It retrieves a timestamp indicating the last time the file was modified.

    namespace Utils
    {
        u32     Strlen(const char* string);
        void*   PushSize(u32 byteCount);
        void*   PushBytes(const void* bytes, u32 byteCount);
        u8*     PushChar(u8 c);
    }
}

#endif // !__FILE_MANAGER_H__