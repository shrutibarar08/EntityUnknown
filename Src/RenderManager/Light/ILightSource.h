#pragma once
#include "SystemManager/PrimaryID.h"
#include <DirectXMath.h>
#include <string>

struct LightDistance
{
	ID id;
	float distance;

	bool operator<(const LightDistance& rhs) const
	{
		return distance < rhs.distance;
	}
};

enum class LightType : uint8_t
{
	Direction_Light,
	Spot_Light,
	Point_Light
};

class ILightSource: public PrimaryID
{
public:
	ILightSource()								= default;
	virtual ~ILightSource()						= default;
	ILightSource(const ILightSource&)				= default;
	ILightSource(ILightSource&&)					= default;
	ILightSource& operator=(const ILightSource&)	= default;
	ILightSource& operator=(ILightSource&&)			= default;

	virtual std::string GetLightName() const = 0;
	virtual LightType GetLightType() const = 0;
};
