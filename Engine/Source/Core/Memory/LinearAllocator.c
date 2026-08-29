#include "LinearAllocator.h"

#include "Core/HAL/LumoraMemory.h"
#include "Core/Logger.h"
#include "Core/Asserts.h"

LUMORA_C_API void CreateLinearAllocator(size_t TotalSize, void* Memory, FLinearAllocator* OutAllocator)
{
    if (OutAllocator)
    {
        OutAllocator->TotalSize = TotalSize;
        OutAllocator->Allocated = 0;
        OutAllocator->bOwnsMemory = (Memory == NULL);

        if (Memory)
        {
            OutAllocator->Memory = Memory;
        }
        else
        {
            OutAllocator->Memory = HAllocate(TotalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}

LUMORA_C_API void ReleaseLinearAllocator(FLinearAllocator* Allocator)
{
    if (Allocator)
    {
        Allocator->Allocated = 0;

        const bool8 bRequiredFreeMemory = Allocator->bOwnsMemory && Allocator->Memory;
        if (bRequiredFreeMemory)
        {
            HFree(Allocator->Memory, Allocator->TotalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }

        Allocator->Memory = NULL;
        Allocator->TotalSize = 0;
        Allocator->bOwnsMemory = FALSE;
    }
}

NODISCARD LUMORA_C_API void* AllocateLinearAllocator(FLinearAllocator* Allocator, size_t Size)
{
    LUMORA_CHECK(Allocator && Allocator->Memory, "AllocateLinearAllocator - provided allocator not initialized.");
    LUMORA_CHECK(Allocator->Allocated <= Allocator->TotalSize, "Linear allocator state corrupted.");

    const size_t Remaining = Allocator->TotalSize - Allocator->Allocated;
    if (Size > Remaining)
    {
        LUMORA_ERROR("AllocateLinearAllocator - Tried to allocate %zuB, only %zuB remaining.", Size, Remaining);
        return NULL;
    }

    void* Block = (uint8*)Allocator->Memory + Allocator->Allocated;
    Allocator->Allocated += Size;
    return Block;
}

LUMORA_C_API void FreeAllLinearAllocator(FLinearAllocator* Allocator)
{
    if (Allocator && Allocator->Memory)
    {
        Allocator->Allocated = 0;
        HZeroMemory(Allocator->Memory, Allocator->TotalSize);
    }
}
