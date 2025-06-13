#pragma once

#include "SystemManager/PrimaryID.h"
#include "Components/ConstantBuffer.h"

#include <DirectXMath.h>
#include <memory>
#include <d3d11.h>


typedef struct CAMERA_INFORMATION_CPU_DESC
{
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMFLOAT3 CameraPosition;
}CAMERA_INFORMATION_DESC;
	
//~ Must attach to slot_1
typedef struct WORLD_TRANSFORM_GPU_DESC
{
	DirectX::XMMATRIX WorldMatrix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMFLOAT3   CameraPosition;
	float               Padding;
}WORLD_TRANSFORM_DESC;

class IRender: public PrimaryID
{
public:
	IRender() = default;
	virtual ~IRender() = default;

	IRender(const IRender&) = default;
	IRender(IRender&&) = default;
	IRender& operator=(const IRender&) = default;
	IRender& operator=(IRender&&) = default;

	virtual bool Build(ID3D11Device* device) = 0;
	virtual bool Render(ID3D11DeviceContext* deviceContext) = 0;

	virtual void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) = 0;
	virtual bool IsInitialized() const = 0;

	void SetScreenWidth(float width);
	void SetScreenHeight(float height);
	// ---- Scale ----
	void SetScale(float x, float y, float z);
	void SetScale(const DirectX::XMFLOAT3& scale);
	void SetScale(const DirectX::XMVECTOR& scale);
	void SetScaleXY(float x, float y);
	void SetScaleXY(const DirectX::XMFLOAT2& scale);
	void SetScaleX(float x);
	void SetScaleY(float y);
	void SetScaleZ(float z);

	void AddScale(float x, float y, float z);
	void AddScale(const DirectX::XMFLOAT3& scale);
	void AddScale(const DirectX::XMVECTOR& scale);
	void AddScale(float x, float y);
	void AddScaleXY(const DirectX::XMFLOAT2& scale);
	void AddScaleX(float x);
	void AddScaleY(float y);
	void AddScaleZ(float z);

	DirectX::XMFLOAT3 GetScale() const;
	DirectX::XMFLOAT2 GetScaleXY() const;
	float GetScaleX() const;
	float GetScaleY() const;
	float GetScaleZ() const;

	// ---- Translation ----
	void SetTranslation(float x, float y, float z);
	void SetTranslation(const DirectX::XMFLOAT3& pos);
	void SetTranslation(const DirectX::XMVECTOR& pos);
	void SetTranslation(float x, float y);
	void SetTranslationXY(const DirectX::XMFLOAT2& pos);
	void SetTranslationX(float x);
	void SetTranslationY(float y);
	void SetTranslationZ(float z);

	void AddTranslation(float x, float y, float z);
	void AddTranslation(const DirectX::XMFLOAT3& pos);
	void AddTranslation(const DirectX::XMVECTOR& pos);
	void AddTranslation(float x, float y);
	void AddTranslationXY(const DirectX::XMFLOAT2& pos);
	void AddTranslationX(float x);
	void AddTranslationY(float y);
	void AddTranslationZ(float z);

	DirectX::XMFLOAT3 GetTranslation() const;
	DirectX::XMFLOAT2 GetTranslationXY() const;
	float GetPositionX() const;
	float GetPositionY() const;
	float GetPositionZ() const;

	// ---- Rotation (in radians) ----
	void SetRotation(float pitch, float yaw, float roll); // standard: pitch=X, yaw=Y, roll=Z
	void SetRotation(const DirectX::XMFLOAT3& rot);
	void SetRotation(const DirectX::XMVECTOR& rot);
	void SetYaw(float yaw);   // Y-axis
	void SetPitch(float pitch); // X-axis
	void SetRoll(float roll); // Z-axis

	void AddRotation(float pitch, float yaw, float roll);
	void AddRotation(const DirectX::XMFLOAT3& rot);
	void AddRotation(const DirectX::XMVECTOR& rot);
	void AddYaw(float yaw);
	void AddPitch(float pitch);
	void AddRoll(float roll);

	DirectX::XMFLOAT3 GetRotation() const;
	float GetYaw() const;
	float GetPitch() const;
	float GetRoll() const;

	DirectX::XMMATRIX GetNormalTransform() const;

protected:
	WORLD_TRANSFORM_DESC m_WorldMatrix{};
	CAMERA_INFORMATION_CPU_DESC m_CameraInformationCPU{};
	DirectX::XMFLOAT3 m_Scale{1.f, 1.f, 1.f};
	DirectX::XMFLOAT3 m_Translation{};
	DirectX::XMFLOAT3 m_Rotation{};
	int m_ScreenWidth{ 1280 };
	int m_ScreenHeight{ 720 };
};
