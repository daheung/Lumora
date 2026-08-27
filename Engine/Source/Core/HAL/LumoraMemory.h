#pragma once

#include "Defines.h"

typedef enum EMemoryTag
{
    /** For temporay use. Should be assigned one of the below or have a new tag created. */
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DYNAMIC_ARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS,
} EMemoryTag;

LUMORA_C_API bool8 InitializeMemory(size_t* const MemoryRequirement, void* State);
LUMORA_C_API void ReleaseMemory(void* State);

LUMORA_C_API void* HAllocate(uint64 AllocSize, EMemoryTag MemoryTag);
LUMORA_C_API void  HFree(void* Block, uint64 AllocSize, EMemoryTag MemoryTag);
LUMORA_C_API void* HZeroMemory(void* Block, uint64 AllocSize);
LUMORA_C_API void* HCopyMemory(void* Dest, const void* Src, uint64 AllocSize);
LUMORA_C_API void* HSetMemory(void* Dest, int32 Value, uint64 AllocSize);

LUMORA_C_API char* HGetMemoryUseageStr();

LUMORA_C_API size_t GetMemoryAllocationCount();