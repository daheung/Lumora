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
void UpdateClock(FClock* Clock);

/** Starts the provided clock. Resets elapsed time. */
void StartClock(FClock* Clock);

/** Stops the provided clock. Does not reset elapsed time. */
void StopClock(FClock* Clock);