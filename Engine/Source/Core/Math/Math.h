 #pragma once

#include "Defines.h"

#define LUMORA_PI				3.14159265358959323846f
#define LUMORA_PI_2				(2.0f * LUMORA_PI)
#define LUMORA_HALF_PI			(0.5f * LUMORA_PI)
#define LUMORA_ONE_OVER_PI		(1.0f / LUMORA_PI)
#define LUMORA_ONE_OVER_TWO_PI	(1.0f / LUMORA_PI_2)

#define LUMORA_SQRT_TWO				1.41421356237309504880f
#define LUMORA_SQRT_THREE			1.73205080756887729352f
#define LUMORA_SQRT_ONE_OVER_TWO	0.70710678118654752440f
#define LUMORA_SQRT_ONE_OVER_THREE	0.57735026918962576451f
#define LUMORA_DEG2RAD_MULTIPLIER	(LUMORA_PI / 180.0f)
#define LUMORA_RAD2DEG_MULTIPLIER	(180.0f / LUMORA_PI)

/** The multiplier to convert seconds to milliseconds. */
#define LUMORA_SEC_TO_MS_MULTIPLIER 1000.f

/** The multiplier to convert milliseconds to seconds. */
#define LUMORA_MS_TO_SEC_MULTIPLIER 0.001f

/** A huge number that should be larger than any valid number used. */
#define LUMORA_INFINITY	1e30f

/** Smallest positive number where 1.0 + FLOAT_EPSILON != 0 */
#define LUMORA_FLOAT_EPSILON 1.192092896e-07f

NODISCARD LUMORA_C_API float32 LumoraSin(float32 X);
NODISCARD LUMORA_C_API float32 LumoraCos(float32 X);
NODISCARD LUMORA_C_API float32 LumoraTan(float32 X);
NODISCARD LUMORA_C_API float32 LumoraAcos(float32 X);
NODISCARD LUMORA_C_API float32 LumoraSqrt(float32 X);
NODISCARD LUMORA_C_API float32 LumoraAbs(float32 X);

/** Indicates if the value if a power of 2. 0 is considering _not_ a power of 2.
 * @param Value The value to ve interpreted.
 * @returns True if the power of 2, otherwise false.
 */
NODISCARD FORCEINLINE bool8 IsPowerOf2(uint64 Value)
{
	return (Value != 0) && ((Value & (Value - 1)) == 0);
}

NODISCARD LUMORA_C_API int32 LumoraRandom();
NODISCARD LUMORA_C_API int32 LumoraRandomInRange(int32 Min, int32 Max);

NODISCARD LUMORA_C_API float32 LumoraFRandom();
NODISCARD LUMORA_C_API float32 LumoraFRandomInRange(float32 Min, float32 Max);

/**
 * @brief Converts provided degrees to radians.
 * 
 * @param Degrees The degrees to be converted.
 * @return The amount in radians.
 */
NODISCARD FORCEINLINE float32 DegToRad(float32 Degrees)
{
	return Degrees * LUMORA_DEG2RAD_MULTIPLIER;
}

/**
 * @brief Converts provided radians to degrees.
 *
 * @param Radians The radians to be converted.
 * @return The amount in degrees.
 */
NODISCARD FORCEINLINE float32 RadToDeg(float32 Radians)
{
	return Radians * LUMORA_RAD2DEG_MULTIPLIER;
}