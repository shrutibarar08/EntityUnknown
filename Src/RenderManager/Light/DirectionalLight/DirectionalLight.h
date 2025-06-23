#pragma once
#include <DirectXMath.h>
#include <string>
#include "RenderManager/Light/ILightSource.h"

typedef struct DIRECTIONAL_LIGHT_GPU_DATA
{
	DirectX::XMFLOAT4 SpecularColor;
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Direction;
	float SpecularPower;
	DirectX::XMMATRIX ViewProjectLightMatrix;
}DIRECTIONAL_LIGHT_GPU_DATA;

class DirectionalLight final: public ILightSource
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
	DIRECTIONAL_LIGHT_GPU_DATA GetLightData() const;
	DirectX::XMFLOAT3 GetLightPosition() const override;

	std::string GetLightName() const override { return "Directional Light"; }
	LightType GetLightType() const override { return LightType::Direction_Light; }
	void UpdateProjectionMatrix(const Frustum& sceneFrustum) override;

private:
	float m_SpecularPower{ 0.f };
	DirectX::XMFLOAT4 m_SpecularColor{};
	DirectX::XMFLOAT4 m_AmbientColor{};
	DirectX::XMFLOAT4 m_DiffuseColor{};
	DirectX::XMFLOAT3 m_Direction{};
};
