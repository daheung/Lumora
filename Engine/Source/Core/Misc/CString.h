#include "Defines.h"

#include <stdarg.h>

LUMORA_C_API bool8 IsPureAscii(const char* Str);

LUMORA_C_API bool8 IsPureAsciiWithLen(const char* Str, const size_t StrLen);

LUMORA_C_API bool8 IsNumeric(const char* Str);

LUMORA_C_API bool8 IsEqual(const char* String1, const char* String2);

LUMORA_C_API char* Strcpy(char* Dest, size_t DestCount, const char* Src);

LUMORA_C_API char* Strncpy(char* Dest, const char* Src, size_t DestCount);

LUMORA_C_API char* Strcat(char* Dest, size_t DestCount, const char* Src);

LUMORA_C_API char* Strncat(char* Dest, const char* Src, int32 MaxLen);

LUMORA_C_API char* Strupr(char* Dest, size_t DestCount);

LUMORA_C_API int32 Strcmp(const char* String1, const char* String2);

LUMORA_C_API int32 Strncmp(const char* String1, const char* String2, size_t Count);

LUMORA_C_API int32 Stricmp(const char* String1, const char* String2);

LUMORA_C_API int32 Strnicmp(const char* String1, const char* String2, size_t Count);

LUMORA_C_API char* Strdup(const char* String);

LUMORA_C_API char* Strndup(const char* String, size_t Count);

LUMORA_C_API const char* Strnistr(const char* Str, int32 InStrLen, const char* Find, int32 FindLen);

LUMORA_C_API const char* Strnstr(const char* Str, int32 InStrLen, const char* Find, int32 FindLen);

LUMORA_C_API size_t Strlen(const char* String);

LUMORA_C_API size_t Strnlen(const char* String, size_t StringSize);

LUMORA_C_API const char* Strstr(const char* String, const char* Find);

LUMORA_C_API char* Strchr(const char* String, char c);

LUMORA_C_API char* Strrchr(char* String, char c);

LUMORA_C_API char* Strrstr(char* String, const char* Find);

LUMORA_C_API int32 Strspn(const char* String, const char* Mask);

LUMORA_C_API int32 Strcspn(const char* String, const char* Mask);

LUMORA_C_API int32 Atoi(const char* String);

LUMORA_C_API int64 Atoi64(const char* String);

LUMORA_C_API float32 Atof(const char* String);

LUMORA_C_API float64 Atod(const char* String);

LUMORA_C_API int32 Strtoi(const char* Start, char** End, int32 Base);

LUMORA_C_API int64 Strtoi64(const char* Start, char** End, int32 Base);

LUMORA_C_API uint64 Strtoui64(const char* Start, char** End, int32 Base);

LUMORA_C_API char* Strtok(char* TokenString, const char* Delim, char** Context);

LUMORA_C_API int32 GetVarArgs(char* Dest, size_t DestSize, const char* Fmt, va_list ArgPtr);

/** Performs string formatting to dest given format string and parameters. */
LUMORA_C_API int32 FormatString(char* Dest, size_t DestSize, const char* Fmt, ...);

/** Performs variadic formatting to dest given format string and va_list.
 * @param Dest The destination for the formatted string.
 * @param Format The string to be formatted.
 * @param ArgPtr The variadic argument list.
 * @returns The size of the data written.
 */
LUMORA_C_API int32 StringFormatV(char* Dest, size_t DestSize, const char* Format, va_list ArgPtr);