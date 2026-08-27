#include "TestManager.h"
#include "Memory/MemoryAllocatorTests.h"

#include <Core/Logger.h>


int main(void)
{
	/** Always initialize the test manager first. */
	TestManagerInit();

	/** TODO: add test registrations here. */
	LinearAllocatorRegisterTests();

	LUMORA_DEBUG("Starting tests.");

	/** Execute tests. */
	TestManagerRunTests();

	LUMORA_DEBUG("Ending tests.");

	return 0;
}