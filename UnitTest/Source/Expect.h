#pragma once

#include "Core/Logger.h"
#include "Core/Math/Math.h"

/** 
 * @brief Exptects Expected to be equal to Actual.
 */
#define ExpectShouldBe(Expected, Actual)																		\
	if (Actual != Expected)																						\
	{																											\
		LUMORA_ERROR("--> Expected %llu, but got: %lld. File: %s:%d.", Expected, Actual, __FILE__, __LINE__);	\
		return FALSE;																							\
	}

/**
 * @brief Exptects Expected to NOT be equal to Actual.
 */
#define ExpectShouldNotBe(Expected, Actual)																				\
	if (Actual == Expected)																								\
	{																													\
		LUMORA_ERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", Expected, Actual, __FILE__, __LINE__);	\
		return FALSE;																									\
	}

/**
 * @brief Exptects Expected to be Actual given a tolerance of LUMORA_FLOAT_EPSILON.
 */
#define ExpectFloatToBe(Expected, Actual)																	\
	if (LumoraAbs(Extected - Actual) > LUMORA_FLOAT_EPSILON)												\
	{																										\
		LUMORA_ERROR("--> Expected %f, but got: %f. File: %s:%d.", Expected, Actual, __FILE__, __LINE__);	\
		return FALSE;																						\
	}

/**
 * @brief Expects Actual to be true.
 */
#define ExpectToBeTrue(Actual)																\
	if (Actual != TRUE)																		\
	{																						\
		LUMORA_ERROR("--> Expected TRUE, but got FALSE. File: %s:%d.", __FILE__, __LINE__);	\
		return FALSE;																		\
	}

