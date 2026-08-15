#pragma once

#include "Defines.h"

/**
 * Memory layout
 * uint64 capacity = number elements that can be held
 * uint64 length = number of elements currently contained
 * uint64 stride = size of each element in bytes
 * void* elements
 */

enum 
{
    ARRAY_CAPACITY,
    ARRAY_LENGTH,
    ARRAY_STRIDE,
    ARRAY_FIELD_LENGTH,
};

/** TODO: Refactoring the type from void* to FArray* for de-referencing the memory operator. */
typedef struct FArray
{
    uint64 Length;
    uint64 Capacity;
    uint64 Stride;
    void* Data;
} FArray;

#define CARRAY_DEFAULT_CAPACITY 1
#define CARRAY_RESIZE_FACTOR 2

LUMORA_C_API void* CArrayCreate(uint64 Sizeof);

LUMORA_C_API void* CArrayCreateWithCapacity(uint64 Sizeof, uint64 Capacity);

LUMORA_C_API void CArrayRelease(void* Array);

LUMORA_C_API void* CArrayResize(void* Array, uint64 OptNewCapacity);

LUMORA_C_API void* CArrayPush(void* Array, void* ValuePtr);

LUMORA_C_API void CArrayPop(void* Array, void* ValuePtr);

LUMORA_C_API void* CArrayInsertAt(void* Array, uint64 Index, void* ValuePtr);

LUMORA_C_API void* CArrayPopAt(void* Array, uint64 Index, void* Dest);

LUMORA_C_API void CArrayClear(void* Array);

LUMORA_C_API uint64 CArrayCapacity(const void* const Array);

LUMORA_C_API uint64 CArrayLength(const void* const Array);

LUMORA_C_API uint64 CArrayStride(const void* const Array);

LUMORA_C_API void CArraySetLength(void* Array, uint64 Value);