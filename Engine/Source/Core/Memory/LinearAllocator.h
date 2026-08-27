#pragma once

#include "Defines.h"


typedef struct FLinearAllocator
{
	size_t TotalSize;
	size_t Allocated;
	void* Memory;
	bool8 bOwnsMemory;
} FLinearAllocator;

LUMORA_C_API void CreateLinearAllocator(size_t TotalSize, void* Memory, FLinearAllocator* OutAllocator);

LUMORA_C_API void ReleaseLinearAllocator(FLinearAllocator* Allocator);

LUMORA_C_API void* AllocateLinearAllocator(FLinearAllocator* Allocator, size_t Size);

LUMORA_C_API void  FreeAllLinearAllocator(FLinearAllocator* Allocator);