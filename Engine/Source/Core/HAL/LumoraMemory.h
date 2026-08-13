#pragma once

#include "Defines.h"

typedef enum EMemoryTag
{
    /** For temporay use. Should be assigned one of the below or have a new tag created. */
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
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

static const char* GMemoryTagStrings[MEMORY_TAG_MAX_TAGS] = 
{
    "UNKNOWN          ",
    "ARRAY            ",
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

LUMORA_C_API void InitializeMemory();
LUMORA_C_API void ReleaseMemory();

LUMORA_C_API void* HAllocate(uint64 AllocSize, EMemoryTag MemoryTag);
LUMORA_C_API void  HFree(void* Block, uint64 AllocSize, EMemoryTag MemoryTag);
LUMORA_C_API void* HZeroMemory(void* Block, uint64 AllocSize);
LUMORA_C_API void* HCopyMemory(void* Dest, const void* Src, uint64 AllocSize);
LUMORA_C_API void* HSetMemory(void* Dest, int32 Value, uint64 AllocSize);

LUMORA_C_API char* HGetMemoryUseageStr();