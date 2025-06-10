#pragma once
#include <DirectXMath.h>
#include "SystemManager/PrimaryID.h"


class DirectionalLight: public PrimaryID
{
public:
	DirectionalLight() = default;
	~DirectionalLight() = default;

	DirectionalLight(const DirectionalLight&) = default;
	DirectionalLight(DirectionalLight&&) = default;
	DirectionalLight& operator=(const DirectionalLight&) = default;
	DirectionalLight& operator=(DirectionalLight&&) = default;

	void SetDiffuseColor(float red, float green, float blue, float alpha);
	void SetDirection(float x, float y, float z);

	DirectX::XMFLOAT4 GetDiffuseColor() const;
	DirectX::XMFLOAT3 GetDirection() const;

private:
	DirectX::XMFLOAT4 m_DiffuseColor;
	DirectX::XMFLOAT3 m_Direction;
};
