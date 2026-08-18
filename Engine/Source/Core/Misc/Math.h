#pragma once

#include "Defines.h"

#define CMathClamp(Value, Min, Max)	\
	(Value < Min) ? Min : (Value > Max ? Max : Value);