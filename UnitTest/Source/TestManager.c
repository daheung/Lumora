#include "TestManager.h"

#include <Core/Containers/Array.h>
#include <Core/Logger.h>
#include <Core/Asserts.h>
#include <Core/Misc/CString.h>
#include <Core/Misc/Clock.h>

typedef struct FTestEntry
{
	FTestFunc TestFunc;
	const char* Desc;
} FTestEntry;

static FTestEntry* GTests;

void TestManagerInit()
{
	GTests = CArrayCreate(sizeof(FTestEntry));
}

void TestManagerRegisterTest(FTestFunc TestFunc, const char* Desc)
{
	FTestEntry Entry = { 0 };
	Entry.TestFunc = TestFunc;
	Entry.Desc = Desc;
	CArrayPush(GTests, &Entry);
}

void TestManagerRunTests()
{
	uint32 Passed  = 0;
	uint32 Failed  = 0;
	uint32 Skipped = 0;

	const uint32 Count = CArrayLength(GTests);

	FClock TotalTime = { 0 };
	StartClock(&TotalTime);

	for (uint32 Index = 0; Index < Count; ++Index)
	{
		FClock TestTime = { 0 };
		StartClock(&TestTime);
		uint8 Result = GTests[Index].TestFunc();
		StopClock(&TestTime);

		if (Result == TRUE)
		{
			++Passed;
		}
		else if (Result == BYPASS)
		{
			LUMORA_WARN("[SKIPPED]: %s", GTests[Index].Desc);
			++Skipped;
		}
		else
		{
			LUMORA_ERROR("[FAILED]: %s", GTests[Index].Desc);
			++Failed;
		}

		char Status[32] = { 0 };
		if (Failed)
		{
			const char Fmt[] = "*** %d FAILED ***";
			FormatString(Status, sizeof(Status), Fmt, Failed);
		}
		else
		{
			const char Fmt[] = "SUCCESS";
			FormatString(Status, sizeof(Status), Fmt, Failed);
		}

		UpdateClock(&TotalTime);
		LUMORA_INFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total)", Index + 1, Count, Skipped, Status, TestTime.ElapsedTime, TotalTime.ElapsedTime);
	}
}
