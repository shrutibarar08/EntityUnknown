#pragma once
#include "Utils/SweetLoader/SweetLoader.h"


class ISystem
{
public:
	ISystem() = default;
	virtual ~ISystem() = default;

	ISystem(const ISystem&) = delete;
	ISystem(ISystem&&) = delete;
	ISystem& operator=(const ISystem&) = delete;
	ISystem& operator=(ISystem&&) = delete;

	//~ Called To Init Configurations
	virtual bool OnInit(const SweetLoader& sweetLoader) = 0;

	//~ Called Every Frame
	virtual bool OnTick(float deltaTime) = 0;

	//~ Save configurations
	virtual bool OnExit(SweetLoader& sweetLoader) = 0;
};
