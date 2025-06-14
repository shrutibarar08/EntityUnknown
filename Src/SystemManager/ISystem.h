#pragma once
#include "PrimaryID.h"
#include "Utils/SweetLoader/SweetLoader.h"


class ISystem: public PrimaryID
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
	virtual bool OnFrameUpdate(float deltaTime) = 0;

	//~ Called Every Frame End
	virtual bool OnFrameEnd() { return true; }

	//~ Save configurations
	virtual bool OnExit(SweetLoader& sweetLoader) = 0;

	virtual std::string GetSystemName() = 0;
};
