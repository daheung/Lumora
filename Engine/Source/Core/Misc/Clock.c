#include "Clock.h"
#include "Platform/Platform.h"

void UpdateClock(FClock* Clock)
{
    if (Clock->StartTime != 0)
    {
        Clock->ElapsedTime = PlatformGetAbsoluteTime() - Clock->StartTime;
    }
}

void StartClock(FClock* Clock)
{
    Clock->StartTime = PlatformGetAbsoluteTime();
    Clock->ElapsedTime = 0;
}

void StopClock(FClock* Clock)
{
    Clock->StartTime = 0;
}