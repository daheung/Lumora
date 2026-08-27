#pragma once

#include "Defines.h"

typedef struct FClock
{
    float64 StartTime;
    float64 ElapsedTime;
} FClock;

/**
 * Updates the provided Clock. Should be called just before checking elapsed time.
 * Has no effect on non-started clocks.
 */
LUMORA_C_API void UpdateClock(FClock* Clock);

/** Starts the provided clock. Resets elapsed time. */
LUMORA_C_API void StartClock(FClock* Clock);

/** Stops the provided clock. Does not reset elapsed time. */
LUMORA_C_API void StopClock(FClock* Clock);