#pragma once

#include "SystemManager/PrimaryID.h"
#include "Components/ConstantBuffer.h"

#include <DirectXMath.h>
#include <memory>
#include <d3d11.h>

#include "Collision/Cube/CubeCollider.h"
#include "RigidBody/RigidBody.h"


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

typedef struct PIXEL_LIGHT_META_GPU
{
	int DirectionalLightCount;
	float Padding[3];
}PIXEL_LIGHT_META_GPU;

class IRender: public PrimaryID
{
public:
	IRender();
	virtual ~IRender()					= default;
	IRender(const IRender&)				= default;
	IRender(IRender&&)					= default;
	IRender& operator=(const IRender&)	= default;
	IRender& operator=(IRender&&)		= default;

	virtual bool Build(ID3D11Device* device) = 0;
	virtual bool Render(ID3D11DeviceContext* deviceContext) = 0;

	virtual void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) = 0;
	virtual bool IsInitialized() const = 0;

	void SetScreenWidth(int width);
	void SetScreenHeight(int height);

	//~ TODO: no time to think or create skeleton class for now (sorry my ego who wanted to code it dynamically)
	CubeCollider* GetCubeCollider() const;
	RigidBody* GetRigidBody();

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

	DirectX::XMMATRIX GetNormalTransform();


protected:
	RigidBody m_RigidBody{};
	std::unique_ptr<CubeCollider> m_CubeCollider{ nullptr };

	WORLD_TRANSFORM_DESC m_WorldMatrix{};
	CAMERA_INFORMATION_CPU_DESC m_CameraInformationCPU{};
	DirectX::XMFLOAT3 m_Scale{1.f, 1.f, 1.f};
	int m_ScreenWidth{ 1280 };
	int m_ScreenHeight{ 720 };
};
