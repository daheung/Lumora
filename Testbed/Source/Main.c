#include <Core/Logger.h>
#include <Core/Asserts.h>

/** TODO: Test */
#include "Platform/Platform.h"

int main(void) {
    InitializeLogging();
    LUMORA_FATAL("A test message: %f", 3.14f);
    LUMORA_ERROR("A test message: %f", 3.14f);
    LUMORA_INFO("A test message: %f", 3.14f);
    LUMORA_DEBUG("A test message: %f", 3.14f);
    LUMORA_TRACE("A test message: %f", 3.14f);

    LUMORA_ASSERT(FALSE);
    
    FPlatformState PlatformState;
    if (PlatformStartup(&PlatformState, "Lumora Engine test", 100, 100, 1280, 720))
    {
        while (TRUE)
        {
            PlatformPumpMessage(&PlatformState);
        }
    }

    PlatformShutdown(&PlatformState);
    ShutdownLogging();
    return 0;
}