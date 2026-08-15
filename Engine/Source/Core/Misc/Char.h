#include "Defines.h"

#include <ctype.h>

/**
* Only converts ASCII characters, same as CRT to[w]upper() with standard C locale
*/
static FORCEINLINE char ToUpper(char Char)
{
    return (char)((uint32)(uint8)(Char) - (((uint32)Char - (uint32)'a' < 26u) << 5));
}

/**
* Only converts ASCII characters, same as CRT to[w]upper() with standard C locale
*/
static FORCEINLINE char ToLower(char Char)
{
    return (char)((uint32)(uint8)(Char) + (((uint32)Char - (uint32)'A' < 26u) << 5));
}

static FORCEINLINE bool8 IsUpper(char Char)       { return isupper ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsLower(char Char)       { return islower ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsAlpha(char Char)       { return isalpha ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsGraph(char Char)       { return isgraph ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsPrint(char Char)       { return isprint ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsPunct(char Char)       { return ispunct ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsAlnum(char Char)       { return isalnum ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsDigit(char Char)       { return isdigit ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsHexDigit(char Char)    { return isxdigit((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsWhitespace(char Char)  { return isspace ((unsigned char)Char) != 0; }
static FORCEINLINE bool8 IsControl(char Char)     { return iscntrl ((unsigned char)Char) != 0; }