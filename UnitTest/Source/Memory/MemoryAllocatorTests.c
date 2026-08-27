#include "MemoryAllocatorTests.h"
#include "TestManager.h"
#include "Expect.h"

#include <Defines.h>
#include <Core/Memory/LinearAllocator.h>

static uint8 LinearAllocatorShouldCreateAndDestroy()
{
	FLinearAllocator Allocator;
	CreateLinearAllocator(sizeof(uint64), NULL, &Allocator);

	ExpectShouldBe(sizeof(uint64), Allocator.TotalSize);
	ExpectShouldBe(0, Allocator.Allocated);
	ExpectShouldNotBe(0, Allocator.Memory);

	ReleaseLinearAllocator(&Allocator);

	ExpectShouldBe(0, Allocator.Memory);
	ExpectShouldBe(0, Allocator.Allocated);
	ExpectShouldBe(0, Allocator.TotalSize);

	return TRUE;
}

static uint8 LinearAllocatorSingleAllocationAllSpace()
{
	FLinearAllocator Allocator = { 0 };
	CreateLinearAllocator(sizeof(size_t), NULL, &Allocator);

	/** Single allocation. */
	void* Block = AllocateLinearAllocator(&Allocator, sizeof(size_t));

	/** Validate it. */
	ExpectShouldNotBe(NULL, Block);
	ExpectShouldBe(sizeof(size_t), Allocator.Allocated);

	ReleaseLinearAllocator(&Allocator);

	return TRUE;
}

static uint8 LinearAllocatorMultiAllocationAllSpace()
{
	const size_t MaxAllocates = 1024L;
	FLinearAllocator Allocator = { 0 };
	CreateLinearAllocator(sizeof(size_t) * MaxAllocates, NULL, &Allocator);

	/** Multiple allocations - full. */
	void* Block = NULL;
	for (size_t Index = 0; Index < MaxAllocates; ++Index)
	{
		Block = AllocateLinearAllocator(&Allocator, sizeof(size_t));
		ExpectShouldNotBe(NULL, Block);
		ExpectShouldBe(sizeof(size_t) * (Index + 1), Allocator.Allocated);
	}

	ReleaseLinearAllocator(&Allocator);

	return TRUE;
}

static uint8 LinearAllocatorMultiAllocationOverAllocate()
{
	const size_t MaxAllocates = 3L;
	FLinearAllocator Allocator = { 0 };
	CreateLinearAllocator(sizeof(size_t) * MaxAllocates, NULL, &Allocator);

	/** Multi allocations - full */
	void* Block = NULL;
	for (size_t Index = 0; Index < MaxAllocates; ++Index)
	{
		Block = AllocateLinearAllocator(&Allocator, sizeof(size_t));

		/** Validate it. */
		ExpectShouldNotBe(NULL, Block);
		ExpectShouldBe(sizeof(size_t) * (Index + 1), Allocator.Allocated);
	}

	LUMORA_DEBUG("Note: The following errir is intentionally caused by this test.");

	/** Ask for one more allocation. SHould error and return 0. */
	Block = AllocateLinearAllocator(&Allocator, sizeof(size_t));

	/** Validate it - allocated should be unchanged. */
	ExpectShouldBe(NULL, Block);
	ExpectShouldBe(sizeof(size_t) * (MaxAllocates), Allocator.Allocated);

	FreeAllLinearAllocator(&Allocator);

	return TRUE;
}

static uint8 LinearAllocatorMultiAllocationAllSpaceThenFree()
{
	const size_t MaxAllocates = 1024L;
	FLinearAllocator Allocator = { 0 };
	CreateLinearAllocator(sizeof(size_t) * MaxAllocates, NULL, &Allocator);

	/** Multiple allocation - full. */
	void* Block = NULL;
	for (size_t Index = 0; Index < MaxAllocates; ++Index)
	{
		Block = AllocateLinearAllocator(&Allocator, sizeof(size_t));

		/** Validate it. */
		ExpectShouldNotBe(NULL, Block);
		ExpectShouldBe(sizeof(size_t) * (Index + 1), Allocator.Allocated);
	}

	/** Validate that pointer is reset. */
	FreeAllLinearAllocator(&Allocator);
	ExpectShouldBe(0, Allocator.Allocated);

	ReleaseLinearAllocator(&Allocator);

	return TRUE;
}

void LinearAllocatorRegisterTests()
{
	const char* DescA = "Linear allocator should create and destroy.";
	TestManagerRegisterTest(LinearAllocatorShouldCreateAndDestroy, DescA);

	const char* DescB = "Linear allocator single allocate for all space.";
	TestManagerRegisterTest(LinearAllocatorSingleAllocationAllSpace, DescB);

	const char* DescC = "Linear allocator multi allocate for all space.";
	TestManagerRegisterTest(LinearAllocatorMultiAllocationAllSpace, DescC);

	const char* DescD = "Linear allocator try over allocate.";
	TestManagerRegisterTest(LinearAllocatorMultiAllocationOverAllocate, DescD);

	const char* DescE = "Linear allocator allocated should be 0 after FreeAllLinearAllocator().";
	TestManagerRegisterTest(LinearAllocatorMultiAllocationAllSpaceThenFree, DescE);
}
