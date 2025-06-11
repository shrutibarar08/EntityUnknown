#pragma once
#include "ILightData.h"
#include <DirectXMath.h>

typedef struct DIRECTIONAL_Light_DATA
{
	DirectX::XMFLOAT4 SpecularColor;
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Direction;
	float SpecularPower;
}DIRECTIONAL_Light_DATA;

class DirectionalLight final: public ILightData<DIRECTIONAL_Light_DATA>
{
public:
	DirectionalLight() = default;
	~DirectionalLight() override = default;

	DirectionalLight(const DirectionalLight&) = default;
	DirectionalLight(DirectionalLight&&) = default;
	DirectionalLight& operator=(const DirectionalLight&) = default;
	DirectionalLight& operator=(DirectionalLight&&) = default;

	void SetAmbient(float red, float green, float blue, float alpha);
	void SetDiffuseColor(float red, float green, float blue, float alpha);
	void SetDirection(float x, float y, float z);
	void SetSpecularColor(float red, float green, float blue, float alpha);
	void SetSpecularPower(float power);

	DirectX::XMFLOAT4 GetAmbientColor() const;
	DirectX::XMFLOAT4 GetDiffuseColor() const;
	DirectX::XMFLOAT4 GetSpecularColor() const;
	DirectX::XMFLOAT3 GetDirection() const;
	float GetSpecularPower() const;
	DIRECTIONAL_Light_DATA GetLightData() const override;
	DirectX::XMFLOAT3 GetLightPosition() const override;

private:
	float m_SpecularPower{ 0.f };
	DirectX::XMFLOAT4 m_SpecularColor;
	DirectX::XMFLOAT4 m_AmbientColor;
	DirectX::XMFLOAT4 m_DiffuseColor;
	DirectX::XMFLOAT3 m_Direction;
};
