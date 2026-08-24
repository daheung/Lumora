#include "Math.h"
#include "Platform/Platform.h"

#include <math.h>
#include <stdlib.h>

static bool8 bRandomSeeded = FALSE;

/**
 * Note that these are here in order to prevent having to import the 
 * entire <math.h> everywhere.
 */

LUMORA_C_API float32 LumoraSin(float32 X)
{
    return sinf(X);
}

LUMORA_C_API float32 LumoraCos(float32 X)
{
    return cosf(X);
}

LUMORA_C_API float32 LumoraTan(float32 X)
{
    return tanf(X);
}

LUMORA_C_API float32 LumoraAcos(float32 X)
{
    return acosf(X);
}

LUMORA_C_API float32 LumoraSqrt(float32 X)
{
    return sqrtf(X);
}

LUMORA_C_API float32 LumoraAbs(float32 X)
{
    return fabsf(X);
}

LUMORA_C_API int32 LumoraRandom()
{
    if (!bRandomSeeded)
    {
        srand((uint32)PlatformGetAbsoluteTime());
        bRandomSeeded = TRUE;
    }
    return rand();
}

LUMORA_C_API int32 LumoraRandomInRange(int32 Min, int32 Max)
{
    if (!bRandomSeeded)
    {
        srand((uint32)PlatformGetAbsoluteTime());
        bRandomSeeded = TRUE;
    }
    return (rand() % (Max  - Min + 1)) + Min;
}

LUMORA_C_API float32 LumoraFRandom()
{
    return (float32)LumoraRandom() / (float32)RAND_MAX;
}

LUMORA_C_API float32 LumoraFRandomInRange(float32 Min, float32 Max)
{
    return Min + ((float32)LumoraRandom() / ((float32)RAND_MAX / (Max - Min)));
}
