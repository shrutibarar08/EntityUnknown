#pragma once
#include "SystemManager/PrimaryID.h"
#include <DirectXMath.h>

class ILightDataBase: public PrimaryID
{
public:
	virtual ~ILightDataBase() = default;

	bool IsDirty() const { return m_Dirty; }
	//~ this unset flag should not be used internally
	void UnSetDirty() { m_Dirty = false; }

protected:
	//~ Set Dirty Whenever Value changes
	void SetDirty() { m_Dirty = true; }

protected:
	bool m_Dirty{ false };
};

template<class T>
class ILightData : public ILightDataBase
{
public:
	ILightData() = default;
	virtual ~ILightData() override = default;

	ILightData(const ILightData&) = default;
	ILightData(ILightData&&) = default;
	ILightData& operator=(const ILightData&) = default;
	ILightData& operator=(ILightData&&) = default;

	virtual T GetLightData() const = 0;
	virtual DirectX::XMFLOAT3 GetLightPosition() const = 0;
};
