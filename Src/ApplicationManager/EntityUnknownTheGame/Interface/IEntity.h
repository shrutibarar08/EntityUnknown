#pragma once
#include "SystemManager/PrimaryID.h"
#include "Utils/SweetLoader/SweetLoader.h"


class IEntity: public virtual PrimaryID
{
public:
	IEntity() = default;
	virtual ~IEntity() = default;
	IEntity(const IEntity&) = default;
	IEntity(IEntity&&) = default;
	IEntity& operator=(const IEntity&) = default;
	IEntity& operator=(IEntity&&) = default;

	virtual void OnBeginPlay(const SweetLoader& config) {}
	virtual void OnTick(float deltaTime) {}
	virtual void SaveSweetData(SweetLoader& sweetLoader) {}
};
