#include "Defines.h"

#include <ctype.h>

/**
* Only converts ASCII characters, same as CRT to[w]upper() with standard C locale
*/
LUMORA_C_API FORCEINLINE char ToUpper(char Char)
{
    return (char)((uint32)(uint8)(Char) - (((uint32)Char - (uint32)'a' < 26u) << 5));
}

/**
* Only converts ASCII characters, same as CRT to[w]upper() with standard C locale
*/
LUMORA_C_API FORCEINLINE char ToLower(char Char)
{
    return (char)((uint32)(uint8)(Char) + (((uint32)Char - (uint32)'A' < 26u) << 5));
}

LUMORA_C_API FORCEINLINE bool8 IsUpper(char Char)       { return iswupper(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsLower(char Char)       { return iswlower(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsAlpha(char Char)       { return iswalpha(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsGraph(char Char)       { return iswgraph(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsPrint(char Char)       { return iswprint(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsPunct(char Char)       { return iswpunct(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsAlnum(char Char)       { return iswalnum(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsDigit(char Char)       { return iswdigit(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsHexDigit(char Char)    { return iswxdigit(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsWhitespace(char Char)  { return iswspace(Char) != 0; }
LUMORA_C_API FORCEINLINE bool8 IsControl(char Char)     { return iswcntrl(Char) != 0; }