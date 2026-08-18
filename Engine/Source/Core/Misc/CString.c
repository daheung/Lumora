#include "CString.h"
#include "Defines.h"
#include "Char.h"
#include "Stricmp.h"

#include "Logger.h"
#include "HAL/LumoraMemory.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

LUMORA_C_API bool8 IsPureAscii(const char* Str)
{
    for (; *Str; ++Str)
    {
        if ((uint8)*Str >= 0x80)
        {
            return FALSE;
        }
    }

    return TRUE;
}

LUMORA_C_API bool8 IsPureAsciiWithLen(const char* Str, const size_t StrLen)
{
    for (size_t Index = 0; Index < StrLen; ++Index, ++Str)
    {
        if ((uint8)*Str >= 0x80)
        {
            return FALSE;
        }
    }

    return TRUE;
}

LUMORA_C_API bool8 IsNumeric(const char* Str)
{
    if (*Str == '-' || *Str == '+')
    {
        Str++;
    }

    bool8 bHasDot = FALSE;
    while (*Str != '\0')
    {
        if (*Str == '.')
        {
            if (bHasDot)
            {
                return FALSE;
            }
            bHasDot = TRUE;
        }
        else if (!IsDigit(*Str))
        {
            return FALSE;
        }

        ++Str;
    }

    return TRUE;
}

/** Case-sensitive string comparison. True if the same, otherwise false. */
LUMORA_C_API bool8 IsEqual(const char *String1, const char *String2)
{
    return Strcmp(String1, String2) == 0;
}

LUMORA_C_API char* Strcpy(char* Dest, size_t DestCount, const char* Src)
{
    LUMORA_UNUSED_PARAM(DestCount);
    return (char*)strcpy(Dest, Src);
}

LUMORA_C_API char* Strncpy(char* Dest, const char* Src, size_t DestCount)
{
    LUMORA_UNUSED_PARAM(DestCount);
    return (char*)strcpy(Dest, Src);
}

LUMORA_C_API char* Strcat(char* Dest, size_t DestCount, const char* Src)
{
    LUMORA_UNUSED_PARAM(DestCount);
    return (char*)strcat(Dest, Src);
}

LUMORA_C_API char* Strncat(char* Dest, const char* Src, int32 MaxLen)
{
    int32 Len = Strlen(Dest);
    char* NewDest = Dest + Len;

    if ((MaxLen -= Len) > 0)
    {
        Strncpy(NewDest, Src, MaxLen);
    }

    return Dest;
}

LUMORA_C_API char* Strupr(char* Dest, size_t DestCount)
{
    for (char* Char = Dest; *Char && DestCount > 0; ++Char, -- DestCount)
    {
        *Char = ToUpper(*Char);
    }

    return Dest;
}

LUMORA_C_API int32 Strcmp(const char* String1, const char* String2)
{
    return (int32)strcmp(String1, String2);
}

LUMORA_C_API int32 Strncmp(const char* String1, const char* String2, size_t Count)
{
    return (int32)strncmp(String1, String2, Count);
}

LUMORA_C_API int32 Stricmp(const char* String1, const char* String2)
{
    return StricmpImpl(String1, String2);
}

LUMORA_C_API int32 Strnicmp(const char* String1, const char* String2, size_t Count)
{
    return StrnicmpImpl(String1, String2, Count);
}

/** Use HFree */
LUMORA_C_API char *Strdup(const char* String)
{
    const size_t Length = Strlen(String);
    char* OutString = HAllocate((Length + 1) * sizeof(char), MEMORY_TAG_STRING);
    HCopyMemory(OutString, String, Length + 1);
    return OutString;
}

LUMORA_C_API char* Strndup(const char* String, size_t Count)
{
    const size_t Length = Strnlen(String, Count);
    char* OutString = HAllocate((Length + 1) * sizeof(char), MEMORY_TAG_STRING);
    HCopyMemory(OutString, String, Length);
    OutString[Length] = '\0';
    return OutString;
}

LUMORA_C_API const char* Strnistr(const char* Str, int32 InStrLen, const char* Find, int32 FindLen)
{
    if (FindLen <= 0)
    {
        LUMORA_LOG(FindLen >= 0, LOG_LEVEL_ERROR, "Invalid FindLen : %d", FindLen);
        return Str;
    }
    if (InStrLen < FindLen)
    {
        LUMORA_LOG(InStrLen >= 0, LOG_LEVEL_ERROR, "Invalid InStrLen : %d", InStrLen);
        return NULL;
    }

    char FindInitial = ToUpper(*Find);
    int32 FindSuffixLength = FindLen - 1;
    const char* FindSuffix = Find + 1;

    const char* StrLastChance = Str + InStrLen - FindLen;
    while (Str <= StrLastChance)
    {
        char StrChar = *Str++;
        StrChar = ToUpper(StrChar);
        if (StrChar == FindInitial && !Strnicmp(Str, FindSuffix, FindSuffixLength))
        {
            return Str - 1;
        }
    }

    return NULL;
}

LUMORA_C_API const char* Strnstr(const char* Str, int32 InStrLen, const char* Find, int32 FindLen)
{
    if (FindLen <= 0)
    {
        LUMORA_LOG(FindLen >= 0, LOG_LEVEL_ERROR, "Invalid FindLen : %d", FindLen);
        return Str;
    }
    if (InStrLen < FindLen)
    {
        LUMORA_LOG(InStrLen >= 0, LOG_LEVEL_ERROR, "Invalid InStrLen : %d", InStrLen);
        return NULL;
    }

    char FindInital = *Find;
    int32 FindSuffixLength = FindLen - 1;
    const char* FindSuffix = Find + 1;
    const char* StrLastChance = Str + InStrLen - FindLen;
    
    while (Str <= StrLastChance)
    {
        char StrChar = *Str++;
        if (StrChar == FindInital && !Strncmp(Str, FindSuffix, FindSuffixLength))
        {
            return Str - 1;
        }
    }

    return NULL;
}

LUMORA_C_API size_t Strlen(const char* String)
{
    return (size_t)strlen(String);
}

LUMORA_C_API size_t Strnlen(const char* String, size_t StringSize)
{
    size_t Length = 0;

    while (Length < StringSize && String[Length] != '\0')
    {
        ++Length;
    }

    return Length;
}

LUMORA_C_API const char* Strstr(const char* String, const char* Find)
{
    return (const char*)strstr(String, Find);
}

LUMORA_C_API char* Strchr(const char* String, char c)
{
    return (char*)strchr(String, c);
}

LUMORA_C_API char* Strrchr(char* String, char c)
{
    return (char*)strrchr(String, c);
}

LUMORA_C_API char* Strrstr(char* String, const char* Find)
{
    if (*Find == (char)0)
    {
        return String + Strlen(String);
    }

    char* Result = NULL;
    for (;;)
    {
        char* Found = (char*)Strstr(String, Find);
        if (!Found)
        {
            return Result;
        }

        Result = Found;
        String = Found + 1;
    }
}

LUMORA_C_API int32 Strspn(const char* String, const char* Mask)
{
    const char* StringIt = String;
    while (*StringIt)
    {
        for (const char* MaskIt = Mask; *MaskIt; ++MaskIt)
        {
            if (*StringIt == *MaskIt)
            {
                goto NextChar;
            }
        }

        return (int32)(StringIt - String);

    NextChar:
        ++StringIt;
    }

    return (int32)(StringIt - String);
}

LUMORA_C_API int32 Strcspn(const char* String, const char* Mask)
{
    const char* StringIt = String;
    while (*StringIt)
    {
        for (const char* MaskIt = Mask; *MaskIt; ++MaskIt)
        {
            if (*StringIt == *MaskIt)
            {
                return (int32)(StringIt - String);
            }
        }

        ++StringIt;
    }

    return (int32)(StringIt - String);
}

LUMORA_C_API int32 Atoi(const char* String)
{
    return atoi(String);
}

LUMORA_C_API int64 Atoi64(const char* String)
{
    return (int64)strtoll(String, NULL, 10);
}

LUMORA_C_API float32 Atof(const char* String)
{
    return strtof(String, NULL);
}

LUMORA_C_API float64 Atod(const char* String)
{
    return strtod(String, NULL);
}

LUMORA_C_API int32 Strtoi(const char* Start, char** End, int32 Base)
{
    return strtol(Start, End, Base);
}

LUMORA_C_API int64 Strtoi64(const char* Start, char** End, int32 Base)
{
    return (int64)strtoll(Start, End, Base);
}

LUMORA_C_API uint64 Strtoui64(const char* Start, char** End, int32 Base)
{
    return (uint64)strtoull(Start, End, Base);
}

LUMORA_C_API char* Strtok(char* TokenString, const char* Delim, char** Context)
{
    char* Current = TokenString ? TokenString : *Context;

    if (!Current)
    {
        return NULL;
    }

    Current += Strspn(Current, Delim);

    if (*Current == '\0')
    {
        *Context = NULL;
        return NULL;
    }

    char* Token = Current;

    Current += Strcspn(Current, Delim);

    if (*Current != '\0')
    {
        *Current = '\0';
        *Context = Current + 1;
    }
    else
    {
        *Context = NULL;
    }

    return Token;
}

LUMORA_C_API int32 GetVarArgs(char* Dest, size_t DestSize, const char* Fmt, va_list ArgPtr)
{
    int32 Result = vsnprintf(Dest, DestSize, Fmt, ArgPtr);
    return Result;
}