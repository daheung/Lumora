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

LUMORA_C_API void* CArrayCreateImpl(uint64 Length, uint64 Stride);
LUMORA_C_API void  CArrayReleaseImpl(void* Array);

LUMORA_C_API uint64 CArrayGetFieldImpl(void* Array, uint64 Field);
LUMORA_C_API void CArraySetFieldImpl(void* Array, uint64 Field, uint64 Value);

LUMORA_C_API void* CArrayResize(void* Array);

LUMORA_C_API void* CArrayPushImpl(void* Array, const void* ValuePtr);
LUMORA_C_API void  CArrayPopImpl(void* Array, void* Dest);

LUMORA_C_API void* CArrayPopAtImpl(void* Array, uint64 Index, void* Dest);
LUMORA_C_API void* CArrayInsertAtImpl(void* Array, uint64 Index, void* ValuePtr);

#define CARRAY_DEFAULT_CAPACITY 1
#define CARRAY_RESIZE_FACTOR 2

LUMORA_C_API FORCEINLINE void* CArrayCreate(uint64 Sizeof)
{
    return CArrayCreateImpl(CARRAY_DEFAULT_CAPACITY, Sizeof);
}

LUMORA_C_API FORCEINLINE void* CArrayCreateWithCapacity(uint64 Sizeof, uint64 Capacity)
{
    return CArrayCreateImpl(Capacity, Sizeof);
}

LUMORA_C_API FORCEINLINE void CArrayRelease(void* Array)
{
    CArrayRelease(Array);
}

#define CArrayPush(Array, Value)            \
{                                           \
    const typeof(Value) Temp = Value;       \
    Array = CArrayPushImpl(Array, &Temp);   \
}
/**
 * NOTE: could use __auto_type for temp above, but intellisense
 * for VSCode flags it as an unknown type. typeof() seems to 
 * work just fine, though. Both are GNU extensions.
 */

LUMORA_C_API FORCEINLINE void CArrayPop(void* Array, void* ValuePtr)
{
    return CArrayPopImpl(Array, ValuePtr);
}

#define CArrayInsertAt(Array, Index, Value)             \
{                                                       \
    typeof(Value) Temp = Value;                         \
    Array = CArrayInsertAtImpl(Array, Index, &Temp);    \
}

LUMORA_C_API FORCEINLINE void* CArrayPopAt(void* Array, uint64 Index, void* Dest)
{
    return CArrayPopAtImpl(Array, Index, Dest);
}

LUMORA_C_API FORCEINLINE void CArrayClear(void* Array)
{
    CArraySetFieldImpl(Array, ARRAY_LENGTH, 0);
}

LUMORA_C_API FORCEINLINE uint64 CArrayCapacity(void* Array)
{
    return CArrayGetFieldImpl(Array, ARRAY_CAPACITY);
}

LUMORA_C_API FORCEINLINE uint64 CArrayLength(void* Array)
{
    return CArrayGetFieldImpl(Array, ARRAY_LENGTH);
}

LUMORA_C_API FORCEINLINE uint64 CArrayStride(void* Array)
{
    return CArrayGetFieldImpl(Array, ARRAY_STRIDE);
}

LUMORA_C_API FORCEINLINE void CArraySetLength(void* Array, uint64 Value)
{
    CArraySetFieldImpl(Array, ARRAY_LENGTH, Value);
}