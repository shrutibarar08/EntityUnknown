#pragma once
#include "IEntity.h"
#include "RenderManager/IRender.h"


class IActor : public IEntity
{
public:
	~IActor() override = default;
	virtual IRender* GetActorMesh() const = 0;
};
