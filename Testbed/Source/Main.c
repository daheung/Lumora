#include <Core/Logger.h>
#include <Core/Asserts.h>

int main(void) {
    InitializeLogging();
    LUMORA_FATAL("A test message: %f", 3.14f);
    LUMORA_ERROR("A test message: %f", 3.14f);
    LUMORA_INFO("A test message: %f", 3.14f);
    LUMORA_DEBUG("A test message: %f", 3.14f);
    LUMORA_TRACE("A test message: %f", 3.14f);

    LUMORA_ASSERT(FALSE);
    
    ShutdownLogging();
    return 0;
}