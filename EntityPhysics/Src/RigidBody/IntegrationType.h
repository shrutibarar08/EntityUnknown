#pragma once
#include <cstdint>


enum class IntegrationType : uint8_t
{
	SemiImplicitEuler,
	Euler,
	Verlet
};
