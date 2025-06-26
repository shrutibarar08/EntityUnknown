#pragma once

class ICollider;

class ForceGenerator
{
public:
	ForceGenerator() = default;
	virtual ~ForceGenerator() = default;

	ForceGenerator(const ForceGenerator&) = default;
	ForceGenerator(ForceGenerator&&) = default;
	ForceGenerator& operator=(ForceGenerator&&) = default;
	ForceGenerator& operator=(const ForceGenerator&) = default;

	virtual void UpdateForce(ICollider* body, float duration) = 0;
};
