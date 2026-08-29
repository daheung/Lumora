#include "FileSystem.h"

#include "Core/Logger.h"
#include "Core/HAL/LumoraMemory.h"
#include "Core/Misc/CString.h"
#include <stdio.h>
#include <sys/stat.h>

LUMORA_C_API bool8 FileSystemExist(const char* Path)
{
    struct stat Buffer;
    return stat(Path, &Buffer) == 0;
}

LUMORA_C_API bool8 FileSystemOpen(const char* Path, EFileModes Mode, bool8 bBinary, FFileHandle* OutHandle)
{
    OutHandle->bIsValid = FALSE;
    OutHandle->Handle = NULL;
    const char* ModeStr = NULL;

    if (Mode & ~(FILE_MODE_READ | FILE_MODE_WRITE))
    {
        ModeStr = bBinary ? "w+b" : "w+";
    }
    else if (Mode & (FILE_MODE_READ | ~FILE_MODE_WRITE))
    {
        ModeStr = bBinary ? "rb" : "r";
    }
    else if (Mode & ~(FILE_MODE_READ | FILE_MODE_WRITE))
    {
        ModeStr = bBinary ? "wb" : "w";
    }
    else
    {
        LUMORA_ERROR("Invalid mode passed while trying to open file: '%s'", Path);
        return FALSE;
    }

    /** Attempt to open the file. */
    FILE* File = fopen(Path, ModeStr);
    if (!File)
    {
        LUMORA_ERROR("Error opening file: '%s'", Path);
        return FALSE;
    }

    OutHandle->Handle = (void*)File;
    OutHandle->bIsValid = TRUE;

    return TRUE;
}

LUMORA_C_API void FileSystemClose(FFileHandle* Handle)
{
    if (Handle->Handle)
    {
        fclose((FILE*)Handle->Handle);
        Handle->Handle = NULL;
        Handle->bIsValid = FALSE;
    }
}

LUMORA_C_API bool8 FileSystemReadLine(FFileHandle* Handle, char** LineBuf)
{
    if (!Handle->Handle)
    {
        return FALSE;
    }

    /** Since we are reading a single line, it should be safe to assume this is enough characters. */
    bool8 bSuccess = FALSE;
    char* Buffer = HAllocate(sizeof(uint8) * 32768, MEMORY_TAG_STRING);
    if (fgets(Buffer, 32768, (FILE*)Handle->Handle) != 0)
    {
        size_t Length = Strlen(Buffer);
        *LineBuf = HAllocate(sizeof(char) * Length + 1, MEMORY_TAG_STRING);
        Strcpy(*LineBuf, 32768, Buffer);
        bSuccess = TRUE;
    }

    HFree(Buffer, sizeof(uint8) * 32768, MEMORY_TAG_STRING);
    return bSuccess;
}

LUMORA_C_API bool8 FileSystemWriteLine(FFileHandle* Handle, const char* Text)
{
    if (!Handle->Handle)
    {
        return FALSE;
    }

    int32 Result = fputs(Text, (FILE*)Handle->Handle);
    if (Result != EOF)
    {
        Result = fputc('\n', (FILE*)Handle->Handle);
    }

    /**
     * Make sure to flush the stream so it is written to the file immediately.
     * This prevents data loss in the event of a crash.
     */
    fflush((FILE*)Handle->Handle);
    return Result != EOF;
}

LUMORA_C_API bool8 FileSystemRead(FFileHandle* Handle, size_t DataSize, void* OutData, size_t* OutBytesRead)
{
    if (!(Handle->Handle && OutData))
    {
        return FALSE;
    }

    *OutBytesRead = fread(OutData, 1, DataSize, (FILE*)Handle->Handle);
    if (*OutBytesRead != DataSize)
    {
        return FALSE;
    }

    return TRUE;
}

LUMORA_C_API bool8 FileSystemReadAllBytes(FFileHandle* Handle, uint8** OutBytes, size_t* OutBytesRead)
{
    if (!Handle->Handle)
    {
        return FALSE;
    }

    fseek((FILE*)Handle->Handle, 0, SEEK_END);
    size_t Size = ftell((FILE*)Handle->Handle);
    rewind((FILE*)Handle->Handle);

    *OutBytes = HAllocate(sizeof(uint8) * Size, MEMORY_TAG_STRING);
    *OutBytesRead = fread(*OutBytes, 1, Size, (FILE*)Handle->Handle);
    if (*OutBytesRead != Size)
    {
        return FALSE;
    }

    return TRUE;
}

LUMORA_C_API bool8 FileSystemWrite(FFileHandle* Handle, size_t DataSize, const void* Data, size_t* OutBytesWritten)
{
    if (!Handle->Handle)
    {
        return FALSE;
    }

    *OutBytesWritten = fwrite(Data, 1, DataSize, (FILE*)Handle->Handle);
    if (*OutBytesWritten != DataSize)
    {
        return FALSE;
    }

    fflush((FILE*)Handle->Handle);
    return TRUE;
}
