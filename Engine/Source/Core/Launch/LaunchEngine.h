#pragma once

#include "Defines.h"

struct FGame;

typedef struct
{
    /** Window starting position x axis, if applicable */
    int16 StartPositionX;

    /** Window starting position y axis, if applicable */
    int16 StartPositionY;

    /** Window starting width, if applicable */
    int16 StartWidth;

    /** Window starting height, if applicable */
    int16 StartHeight;

    /** The application name used in windowing, if applicable */
    char* ApplicationName;
} FApplicationConfig;

LUMORA_C_API bool8 ApplicationCreate(struct FGame* GameInstance);

LUMORA_C_API bool8 ApplocationLoop();

void ApplicationGetFrameBufferSize(uint32* Width, uint32* Height);