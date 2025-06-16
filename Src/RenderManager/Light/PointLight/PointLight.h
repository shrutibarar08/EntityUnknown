#pragma once
#include <DirectXMath.h>
#include <string>

#include "RenderManager/Light/ILightSource.h"

typedef struct POINT_LIGHT_GPU_DATA
{
    DirectX::XMFLOAT4 SpecularColor;
    DirectX::XMFLOAT4 AmbientColor;
    DirectX::XMFLOAT4 DiffuseColor;

    DirectX::XMFLOAT3 Position;
    float Range;

    float SpecularPower;
    float Padding[3]; // For 16-byte alignment
} POINT_LIGHT_GPU_DATA;

class PointLight final : public ILightSource
{
public:
    PointLight() = default;
    ~PointLight() override = default;

    PointLight(const PointLight&) = default;
    PointLight(PointLight&&) = default;
    PointLight& operator=(const PointLight&) = default;
    PointLight& operator=(PointLight&&) = default;

    void SetAmbient(float r, float g, float b, float a);
    void SetDiffuseColor(float r, float g, float b, float a);
    void SetSpecularColor(float r, float g, float b, float a);
    void SetSpecularPower(float power);

    void SetPosition(float x, float y, float z);
    void SetRange(float range);

    DirectX::XMFLOAT4 GetAmbientColor() const;
    DirectX::XMFLOAT4 GetDiffuseColor() const;
    DirectX::XMFLOAT4 GetSpecularColor() const;
    DirectX::XMFLOAT3 GetPosition() const;
    float GetRange() const;
    float GetSpecularPower() const;

    POINT_LIGHT_GPU_DATA GetLightData() const;
    DirectX::XMFLOAT3 GetLightPosition() const;

    std::string GetLightName() const override { return "Point Light"; }
    LightType GetLightType() const override { return LightType::Point_Light; }

private:
    float m_SpecularPower{ 32.f };
    float m_Range{ 10.f };

    DirectX::XMFLOAT4 m_SpecularColor{};
    DirectX::XMFLOAT4 m_AmbientColor{};
    DirectX::XMFLOAT4 m_DiffuseColor{};

    DirectX::XMFLOAT3 m_Position{};
};
