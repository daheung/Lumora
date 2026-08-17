#include "Containers/Array.h"

#include "HAL/LumoraMemory.h"
#include "Asserts.h"
#include "Logger.h"

static FORCEINLINE void* CArrayCreateImpl(uint64 Length, uint64 Stride);
static FORCEINLINE void  CArrayReleaseImpl(void* Array);
static FORCEINLINE uint64 CArrayGetFieldImpl(const void* const Array, uint64 Field);
static FORCEINLINE void CArraySetFieldImpl(void* Array, uint64 Field, uint64 Value);
static FORCEINLINE void* CArrayResizeImpl(void* Array, uint64 OptNewCapacity);
static FORCEINLINE void CArrayPopImpl(void* RESTRICT Array, void* RESTRICT Dest);
static FORCEINLINE void* CArrayPopAtImpl(void* RESTRICT Array, uint64 Index, void* RESTRICT Dest);
static FORCEINLINE void* CArrayInsertAtImpl(void* Array, uint64 Index, void* ValuePtr);

LUMORA_C_API void* CArrayCreate(uint64 Sizeof)
{
    return CArrayCreateImpl(CARRAY_DEFAULT_CAPACITY, Sizeof);
}

LUMORA_C_API void* CArrayCreateWithCapacity(uint64 Sizeof, uint64 Capacity)
{
    return CArrayCreateImpl(Capacity, Sizeof);
}

LUMORA_C_API void CArrayRelease(void* Array)
{
    CArrayReleaseImpl(Array);
}

LUMORA_C_API void* CArrayResize(void* Array, uint64 OptNewCapacity)
{
    return CArrayResizeImpl(Array, OptNewCapacity);
}

LUMORA_C_API void CArrayPop(void* Array, void* ValuePtr)
{
    CArrayPopImpl(Array, ValuePtr);
}

LUMORA_C_API void* CArrayInsertAt(void* Array, uint64 Index, void* ValuePtr)
{
    return CArrayInsertAtImpl(Array, Index, ValuePtr);
}

LUMORA_C_API void* CArrayPopAt(void* Array, uint64 Index, void* Dest)
{
    return CArrayPopAtImpl(Array, Index, Dest);
}

LUMORA_C_API void CArrayClear(void* Array)
{
    CArraySetFieldImpl(Array, ARRAY_LENGTH, 0);
}

LUMORA_C_API uint64 CArrayCapacity(const void* const Array)
{
    return CArrayGetFieldImpl(Array, ARRAY_CAPACITY);
}

LUMORA_C_API uint64 CArrayLength(const void* const Array)
{
    return CArrayGetFieldImpl(Array, ARRAY_LENGTH);
}

LUMORA_C_API uint64 CArrayStride(const void* const Array)
{
    return CArrayGetFieldImpl(Array, ARRAY_STRIDE);
}

LUMORA_C_API void CArraySetLength(void* Array, uint64 Value)
{
    CArraySetFieldImpl(Array, ARRAY_LENGTH, Value);
}



void* CArrayCreateImpl(uint64 Length, uint64 Stride)
{
    uint64 HeaderSize = ARRAY_FIELD_LENGTH * sizeof(uint64);
    uint64 ArraySize = Length * Stride;
    uint64* OutArray = HAllocate(HeaderSize + ArraySize, MEMORY_TAG_DYNAMIC_ARRAY);
    HSetMemory(OutArray, 0, HeaderSize + ArraySize);

    OutArray[ARRAY_CAPACITY] = Length;
    OutArray[ARRAY_LENGTH] = 0;
    OutArray[ARRAY_STRIDE] = Stride;

    return (void*)(OutArray + ARRAY_FIELD_LENGTH);
}

void CArrayReleaseImpl(void* Array)
{
    uint64* Header = (uint64*)Array - ARRAY_FIELD_LENGTH;
    uint64 HeaderSize = ARRAY_FIELD_LENGTH * sizeof(uint64);
    uint64 TotalSize = HeaderSize + Header[ARRAY_CAPACITY] * Header[ARRAY_STRIDE];
    HFree(Header, TotalSize, MEMORY_TAG_DYNAMIC_ARRAY);
}

uint64 CArrayGetFieldImpl(const void* const Array, uint64 Field)
{
    uint64* Header = (uint64*)Array - ARRAY_FIELD_LENGTH;
    return Header[Field];
}

void CArraySetFieldImpl(void* Array, uint64 Field, uint64 Value)
{
    uint64* Header = (uint64*)Array - ARRAY_FIELD_LENGTH;
    Header[Field] = Value;
}

/** TODO: Use OptNewCapacity Param */
void* CArrayResizeImpl(void* Array, uint64 OptNewCapacity)
{
    uint64 Length = CArrayLength(Array);
    uint64 Stride = CArrayStride(Array);
    void* NewArray = CArrayCreateImpl((CARRAY_RESIZE_FACTOR * CArrayCapacity(Array)), Stride);
    HCopyMemory(NewArray, Array, Length * Stride);

    CArraySetFieldImpl(NewArray, ARRAY_LENGTH, Length);
    CArrayRelease(Array);
    return NewArray;
}

LUMORA_C_API void* CArrayPushImpl(void *Array, const void *ValuePtr)
{
    uint64 Length = CArrayLength(Array);
    uint64 Stride = CArrayStride(Array);
    if (Length >= CArrayCapacity(Array))
    {
        Array = CArrayResize(Array, 0);
    }

    const uint8* Start = (uint8*)Array;
    const uint8* Data = Start + Length * Stride;
    HCopyMemory((void*)Data, ValuePtr, Stride);

    CArraySetFieldImpl(Array, ARRAY_LENGTH, Length + 1);
    return Array;
}

void CArrayPopImpl(void* RESTRICT Array, void* RESTRICT Dest)
{
    LUMORA_ASSERT_MSG(Array, "Array is null.");
    LUMORA_ASSERT_MSG(Dest, "Destination is null.");

    uint64 Length = CArrayLength(Array);
    uint64 Stride = CArrayStride(Array);

    LUMORA_ASSERT_MSG(Length > 0, "Array length less than 0.");

    const uint8* Start = (uint8*)Array;
    const uint8* Data = Start + (Length - 1) * Stride; 
    HCopyMemory(Dest, Data, Stride);
    CArraySetFieldImpl(Array, ARRAY_LENGTH, Length - 1);
}

void* CArrayPopAtImpl(void* RESTRICT Array, uint64 Index, void* RESTRICT Dest)
{
    LUMORA_ASSERT_MSG(Array, "Array is null.");
    LUMORA_ASSERT_MSG(Dest, "Destination is null.");

    uint64 Length = CArrayLength(Array);
    uint64 Stride = CArrayStride(Array);

    LUMORA_LOG(Index < Length, LOG_LEVEL_FATAL, "Index outside the bounds of this array. Length : %i, index : %iindex", Length, Index);
    
    const uint8* Start = (uint8*)Array;
    const uint8* Data = Start + Length * Stride; 
    HCopyMemory(Dest, Data, Stride);

    /** If not on the last element, snip out the entry and copy the rest inward. */
    if (Index != Length - 1)
    {
        HCopyMemory((void*)Data, Data + 1, Stride * (Length - Index));
    }

    CArraySetFieldImpl(Array, ARRAY_LENGTH, Length - 1);
    return Array;
}

void* CArrayInsertAtImpl(void *Array, uint64 Index, void *ValuePtr)
{
    LUMORA_ASSERT_MSG(Array, "Array is null.");
    LUMORA_ASSERT_MSG(ValuePtr, "Destination is null.");

    const uint64 Length = CArrayLength(Array);
    const uint64 Stride = CArrayStride(Array);
    const uint64 Capacity = CArrayCapacity(Array);

    LUMORA_LOG(Index < Length, LOG_LEVEL_FATAL, "Index outside the bounds of this array. Length : %i, index : %iindex", Length, Index);
    
    if (Capacity < Length - 1)
    {
        Array = CArrayResize(Array, 0);
    }

    const uint8* Start = (uint8*)Array;
    const uint8* Data = Start + Length * Stride;

    /** If not on the last element, snip out the entry and copy the rest inward. */
    if (Index != Length - 1)
    {
        HCopyMemory((void*)(Data + 1), Data, Stride * (Length - Index));
    }

    /** Set the value at the index. */
    HCopyMemory((void*)Data, ValuePtr, Stride);

    CArraySetFieldImpl(Array, ARRAY_LENGTH, Length + 1);
    return Array;
}
