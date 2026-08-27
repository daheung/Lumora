#pragma once

#include "Defines.h"

#define BYPASS 2

typedef uint8(*FTestFunc)();

void TestManagerInit();

void TestManagerRegisterTest(FTestFunc TestFunc, const char* Desc);

void TestManagerRunTests();