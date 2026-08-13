#include "Containers/Array.h"

#include "HAL/LumoraMemory.h"
#include "Asserts.h"

static FORCEINLINE void* CopyToEmpty(void* NewArray, const void* PrevArray, uint64 PrevMax);
static FORCEINLINE void* CopyToEmptyWithSlack(void* NewArray, const void* PrevArray, uint64 PrevMax, uint64 ExtraSlack);

static FORCEINLINE void* CopyToEmpty(void* NewArray, const void* PrevArray, uint64 PrevMax)
{
    return CopyToEmptyWithSlack(NewArray, PrevArray, PrevMax, 0);
}

/**
 * Copies data from one array into this array. Uses the fast path if the
 * data in question does not need a constructor.
 *
 * @param Source The source array to copy
 * @param PrevMax The previous allocated size
 * @param ExtraSlack Additional amount of memory to allocate at
 *                   the end of the buffer. Counted in elements.
 */
static FORCEINLINE void* CopyToEmptyWithSlack(void* NewArray, const void* PrevArray, uint64 PrevMax, uint64 ExtraSlack)
{
    uint64 Length = CArrayLength(PrevArray);
    uint64 NewCapacity = Length + ExtraSlack;

    uint64 Stride = CArrayStride(PrevArray);
    uint64 Capacity = CArrayCapacity(PrevArray);
    HCopyMemory(NewArray, PrevArray, PrevMax);

    CArraySetFieldImpl(NewArray, ARRAY_LENGTH, Length);
    CArraySetFieldImpl(NewArray, ARRAY_STRIDE, Stride);
    CArraySetFieldImpl(NewArray, ARRAY_CAPACITY, Capacity);
    return NewArray;
}

static FORCEINLINE uint32 GetInitCapacity()
{
    return 0;
}

static FORCEINLINE uint32 DefaultCalculateSlackGrow(uint32 NumElements, uint32 NumAllocatedElements, size_t BytesPerElement)
{

}


LUMORA_C_API void* CArrayCreateImpl(uint64 Length, uint64 Stride)
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

LUMORA_C_API void CArrayReleaseImpl(void *Array)
{
    uint64* Header = (uint64*)Array - ARRAY_FIELD_LENGTH;
    uint64 HeaderSize = ARRAY_FIELD_LENGTH * sizeof(uint64);
    uint64 TotalSize = HeaderSize + Header[ARRAY_CAPACITY] * Header[ARRAY_STRIDE];
    HFree(Header, TotalSize, MEMORY_TAG_DYNAMIC_ARRAY);
}

LUMORA_C_API uint64 CArrayGetFieldImpl(void *Array, uint64 Field)
{
    uint64* Header = (uint64*)Array - ARRAY_FIELD_LENGTH;
    return Header[Field];
}

LUMORA_C_API void CArraySetFieldImpl(void *Array, uint64 Field, uint64 Value)
{
    uint64* Header = (uint64*)Array - ARRAY_FIELD_LENGTH;
    Header[Field] = Value;
}

/** TODO: Use OptNewCapacity Param */
LUMORA_C_API void* CArrayResize(void *Array, uint64 OptNewCapacity)
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
        Array = CArrayResize(Array, NULL);
    }

    const uint8* Start = (uint8*)Array;
    const uint8* Data = Start + Length * Stride;
    HCopyMemory((void*)Data, ValuePtr, Stride);

    CArraySetFieldImpl(Array, ARRAY_LENGTH, Length + 1);
    return Array;
}

LUMORA_C_API void CArrayPopImpl(void* RESTRICT Array, void* RESTRICT Dest)
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

LUMORA_C_API void *CArrayPopAtImpl(void* RESTRICT Array, uint64 Index, void* RESTRICT Dest)
{
    LUMORA_ASSERT_MSG(Array, "Array is null.");
    LUMORA_ASSERT_MSG(Dest, "Destination is null.");

    uint64 Length = CArrayLength(Array);
    uint64 Stride = CArrayStride(Array);

    LUMORA_ASSERT_MSG(Index < Length, "Index outside the bounds of this array. Length : %i, index : %iindex", Length, Index);
    
    const uint8* Start = (uint8*)Array;
    const uint8* Data = Start + Length * Stride; 
    HCopyMemory(Dest, Data, Stride);

    /** If not on the last element, snip out the entry and copy the rest inward. */
    if (Index != Length - 1)
    {
        HCopyMemory(Data, Data + 1, Stride * (Length - Index));
    }

    CArraySetFieldImpl(Array, ARRAY_LENGTH, Length - 1);
    return Array;
}

LUMORA_C_API void *CArrayInsertAtImpl(void *Array, uint64 Index, void *ValuePtr)
{
    LUMORA_ASSERT_MSG(Array, "Array is null.");
    LUMORA_ASSERT_MSG(ValuePtr, "Destination is null.");

    const uint64 Length = CArrayLength(Array);
    const uint64 Stride = CArrayStride(Array);
    const uint64 Capacity = CArrayCapacity(Array);

    LUMORA_ASSERT_MSG(Index < Length, "Index outside the bounds of this array. Length : %i, index : %iindex", Length, Index);
    
    if (Capacity < Length - 1)
    {
        Array = CArrayResize(Array, NULL);
    }

    const uint8* Start = (uint8*)Array;
    const uint8* Data = Start + Length * Stride;

    /** If not on the last element, snip out the entry and copy the rest inward. */
    if (Index != Length - 1)
    {
        HCopyMemory(Data + 1, Data, Stride * (Length - Index));
    }

    /** Set the value at the index. */
    HCopyMemory(Data, ValuePtr, Stride);

    CArraySetFieldImpl(Array, ARRAY_LENGTH, Length + 1);
    return Array;
}
