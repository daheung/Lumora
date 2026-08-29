#include "LumoraMemory.h"

#include "Core/Logger.h"
#include "Core/Asserts.h"
#include "Platform/Platform.h"
#include "Misc/CString.h"

/** TODO: Custom string lib */
#include <string.h>
#include <stdio.h>

static const char* GMemoryTagStrings[MEMORY_TAG_MAX_TAGS] =
{
    "UNKNOWN          ",
    "ARRAY            ",
    "LINEAR_ALLOCATOR ",
    "DYNAMIC_ARRAY    ",
    "DICT             ",
    "RING_QUEUE       ",
    "BST              ",
    "STRING           ",
    "APPLICATION      ",
    "JOB              ",
    "TEXTURE          ",
    "MATERIAL_INSTANCE",
    "RENDERER         ",
    "GAME             ",
    "TRANSFORM        ",
    "ENTITY           ",
    "ENTITY_NODE      ",
    "SCENE            ",
};

typedef struct FMemoryStats
{
    uint64 TotalAllocated;
    uint64 TaggedAllocations[MEMORY_TAG_MAX_TAGS];
} FMemoryStats;

typedef struct FMemorySystemState
{
    FMemoryStats GMemoryStats;
    size_t AllocationCount;
} FMemorySystemState;

static FMemorySystemState* GMemorySystemState;

void InitializeMemory(size_t* const MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(FMemorySystemState);
    if (State == NULL)
    {
        return;
    }

    GMemorySystemState = State;
    GMemorySystemState->AllocationCount = 0;
    PlatformZeroMemory(&GMemorySystemState->GMemoryStats, sizeof(FMemoryStats));
}

void ReleaseMemory(void* State)
{
    GMemorySystemState = NULL;
    // PlatformFree(&GMemoryStats, FALSE);
}

LUMORA_C_API void* HAllocate(uint64 AllocSize, EMemoryTag MemoryTag)
{
    LUMORA_LOG(MemoryTag != MEMORY_TAG_UNKNOWN , LOG_LEVEL_WARN, "HAllocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    LUMORA_CHECK(MemoryTag != MEMORY_TAG_MAX_TAGS, "Invalid MemoryTag Param");

    if (GMemorySystemState)
    {
        GMemorySystemState->GMemoryStats.TotalAllocated += AllocSize;
        GMemorySystemState->GMemoryStats.TaggedAllocations[MemoryTag] += AllocSize;
        GMemorySystemState->AllocationCount++;
    }

    /** TODO: Memory Alignment */
    void* Block = PlatformAllocate(AllocSize, FALSE);
    PlatformZeroMemory(Block, AllocSize);

    return Block;
}

LUMORA_C_API void HFree(void* Block, uint64 AllocSize, EMemoryTag MemoryTag)
{
    LUMORA_LOG(MemoryTag != MEMORY_TAG_UNKNOWN, LOG_LEVEL_WARN, "HFree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    LUMORA_LOG(MemoryTag != MEMORY_TAG_MAX_TAGS, LOG_LEVEL_FATAL, "Invalid MemoryTag Param");

    LUMORA_ASSERT_MSG(AllocSize <= GMemorySystemState->GMemoryStats.TotalAllocated, "AllocSize cannot exceed TotalAllocated.");
    LUMORA_ASSERT_MSG(AllocSize <= GMemorySystemState->GMemoryStats.TaggedAllocations[MemoryTag],"AllocSize cannot exceed TaggedAllocation.");

    GMemorySystemState->GMemoryStats.TotalAllocated -= AllocSize;
    GMemorySystemState->GMemoryStats.TaggedAllocations[MemoryTag] -= AllocSize;

    /** TODO: Memory Alignment */
    PlatformFree(Block, FALSE);
}

LUMORA_C_API void* HZeroMemory(void *Block, uint64 AllocSize)
{
    return PlatformZeroMemory(Block, AllocSize);
}

LUMORA_C_API void* HCopyMemory(void *Dest, const void *Src, uint64 AllocSize)
{
    return PlatformCopyMemory(Dest, Src, AllocSize);
}

LUMORA_C_API void* HSetMemory(void *Dest, int32 Value, uint64 AllocSize)
{
    return PlatformSetMemory(Dest, Value, AllocSize);
}

LUMORA_C_API char* HGetMemoryUseageStr()
{
    const uint64 Gib = 1024 * 1024 * 1024;
    const uint64 Mib = 1024 * 1024;
    const uint64 Kib = 1024;

    char Buffer[1024] = "System memory use (tagged): \n";
    uint64 Offset = strlen(Buffer);

    for (uint32 TagIndex = 0; TagIndex < MEMORY_TAG_MAX_TAGS; ++TagIndex)
    {
        char Unit[4] = "_iB";
        float32 Amount = 10.f;

        if (GMemorySystemState->GMemoryStats.TaggedAllocations[TagIndex] >= Gib)
        {
            Unit[0] = 'G';
            Amount = GMemorySystemState->GMemoryStats.TaggedAllocations[TagIndex] / (float32)Gib;
        } 
        else if (GMemorySystemState->GMemoryStats.TaggedAllocations[TagIndex] >= Mib)
        {
            Unit[0] = 'M';
            Amount = GMemorySystemState->GMemoryStats.TaggedAllocations[TagIndex] / (float32)Mib;
        }
        else if (GMemorySystemState->GMemoryStats.TaggedAllocations[TagIndex] >= Mib)
        {
            Unit[0] = 'K';
            Amount = GMemorySystemState->GMemoryStats.TaggedAllocations[TagIndex] / (float32)Kib;
        } 
        else
        {
            HCopyMemory(Unit, "B\0", sizeof("B\0"));
            Amount = (float32)GMemorySystemState->GMemoryStats.TaggedAllocations[TagIndex];
        }

        int32 Length = snprintf(Buffer + Offset, 1024, "  %s: %.2f%s\n", GMemoryTagStrings[TagIndex], Amount, Unit);
        Offset += Length;
    }

    char* OutString = Strdup(Buffer);
    return OutString;
}

LUMORA_C_API size_t GetMemoryAllocationCount()
{
    if (GMemorySystemState)
    {
        return GMemorySystemState->AllocationCount;
    }

    return (size_t)0;
}
