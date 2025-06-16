#pragma once
#include <DirectXMath.h>
#include <string>

#include "RenderManager/Light/ILightSource.h"

typedef struct SPOT_LIGHT_GPU_DATA
{
	DirectX::XMFLOAT4 SpecularColor;
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Direction;
	float SpotAngle; // Cosine of outer cone angle (e.g., cos(0))

	float SpecularPower;
	float Padding[3]; // For 16-byte alignment
} SPOT_LIGHT_GPU_DATA;

class SpotLight final: public ILightSource
{
public:
	SpotLight() = default;
	~SpotLight() override = default;

	SpotLight(const SpotLight&) = default;
	SpotLight(SpotLight&&) = default;
	SpotLight& operator=(const SpotLight&) = default;
	SpotLight& operator=(SpotLight&&) = default;

	void SetAmbient(float r, float g, float b, float a);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);

	void SetPosition(float x, float y, float z);
	void SetDirection(float x, float y, float z);
	void SetRange(float range);
	void SetSpotAngleDegrees(float degrees); // auto converts to cos(radians)

	DirectX::XMFLOAT4 GetAmbientColor() const;
	DirectX::XMFLOAT4 GetDiffuseColor() const;
	DirectX::XMFLOAT4 GetSpecularColor() const;
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetDirection() const;
	float GetRange() const;
	float GetSpotAngleDegree() const;
	float GetSpecularPower() const;

	SPOT_LIGHT_GPU_DATA GetLightData() const;
	DirectX::XMFLOAT3 GetLightPosition() const;

	std::string GetLightName() const override { return "Spot Light"; }
	LightType GetLightType() const override { return LightType::Spot_Light; }

private:
	float m_SpecularPower{ 32.f };
	float m_Range{ 10.f };
	float m_SpotAngleDegree{ 34.f };

	DirectX::XMFLOAT4 m_SpecularColor{};
	DirectX::XMFLOAT4 m_AmbientColor{};
	DirectX::XMFLOAT4 m_DiffuseColor{};

	DirectX::XMFLOAT3 m_Position{};
	DirectX::XMFLOAT3 m_Direction{};
};
