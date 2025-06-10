#pragma once
#include "SystemManager/PrimaryID.h"


typedef struct CAMERA_MATRIX_DESC
{
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
}CAMERA_MATRIX_DESC;

typedef struct WORLD_TRANSFORM
{
	DirectX::XMMATRIX TransformationMatrix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
}WORLD_TRANSFORM;

typedef struct DIRECTIONAL_LIGHT_CB
{
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 LightDirection;
	float Padding;
}DIRECTIONAL_LIGHT_CB;

class IModel: public PrimaryID
{
public:
	IModel() = default;
	virtual ~IModel() = default;

	virtual bool IsInitialized() const = 0;
	virtual bool Build(ID3D11Device* device) = 0;
	virtual bool Render(ID3D11DeviceContext* deviceContext) = 0;
	virtual void UpdateTransformation(const CAMERA_MATRIX_DESC* cameraInfo) = 0;
	virtual void UpdateDirectionalLight(const DIRECTIONAL_LIGHT_CB* lightInfo) = 0;

	WORLD_TRANSFORM GetWorldTransform() const { return m_WorldTransform; }

	DirectX::XMVECTOR GetTranslationVector() const
	{
		return DirectX::XMVectorSet(m_TranslationX, m_TranslationY, m_TranslationZ, 1.0f);
	}

	DirectX::XMVECTOR GetRotationVector() const
	{
		return DirectX::XMVectorSet(m_RotationPitch, m_RotationYaw, m_RotationRoll, 1.0f);
	}

	DirectX::XMVECTOR GetScaleVector() const
	{
		return DirectX::XMVectorSet(m_ScaleX, m_ScaleY, m_ScaleZ, 1.0f);
	}

	DirectX::XMMATRIX GetTransform() const
	{
		using namespace DirectX;

		XMMATRIX scaleMat = XMMatrixScaling(m_ScaleX, m_ScaleY, m_ScaleZ);
		XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_RotationPitch, m_RotationYaw, m_RotationRoll);
		XMMATRIX transMat = XMMatrixTranslation(m_TranslationX, m_TranslationY, m_TranslationZ);

		return XMMatrixTranspose(scaleMat * rotMat * transMat);
	}

	// --- Scale Set/Add ---
	void SetScale(float x, float y, float z) { m_ScaleX = x; m_ScaleY = y; m_ScaleZ = z; }
	void SetScaleX(float x) { m_ScaleX = x; }
	void SetScaleY(float y) { m_ScaleY = y; }
	void SetScaleZ(float z) { m_ScaleZ = z; }

	void AddScale(float x, float y, float z) { m_ScaleX += x; m_ScaleY += y; m_ScaleZ += z; }
	void AddScaleX(float x) { m_ScaleX += x; }
	void AddScaleY(float y) { m_ScaleY += y; }
	void AddScaleZ(float z) { m_ScaleZ += z; }

	// --- Rotation Set/Add ---
	void SetRotation(float pitch, float yaw, float roll)
	{
		m_RotationPitch = pitch;
		m_RotationYaw = yaw;
		m_RotationRoll = roll;
	}

	void SetPitch(float pitch) { m_RotationPitch = pitch; }
	void SetYaw(float yaw) { m_RotationYaw = yaw; }
	void SetRoll(float roll) { m_RotationRoll = roll; }

	void AddPitch(float delta) { m_RotationPitch += delta; }
	void AddYaw(float delta) { m_RotationYaw += delta; }
	void AddRoll(float delta) { m_RotationRoll += delta; }

	// --- Translation Set/Add ---
	void SetTranslation(float x, float y, float z) { m_TranslationX = x; m_TranslationY = y; m_TranslationZ = z; }
	void SetTranslationX(float x) { m_TranslationX = x; }
	void SetTranslationY(float y) { m_TranslationY = y; }
	void SetTranslationZ(float z) { m_TranslationZ = z; }

	void AddTranslation(float x, float y, float z) { m_TranslationX += x; m_TranslationY += y; m_TranslationZ += z; }
	void AddTranslationX(float x) { m_TranslationX += x; }
	void AddTranslationY(float y) { m_TranslationY += y; }
	void AddTranslationZ(float z) { m_TranslationZ += z; }

protected:
	WORLD_TRANSFORM m_WorldTransform{};
	DIRECTIONAL_LIGHT_CB m_LightTransform{};
	// Individual float components (replaces XMVECTORs)
	float m_TranslationX = 0.0f;
	float m_TranslationY = 0.0f;
	float m_TranslationZ = 5.0f;

	float m_RotationPitch = 0.0f;
	float m_RotationYaw = 0.0f;
	float m_RotationRoll = 0.f;

	float m_ScaleX = 1.0f;
	float m_ScaleY = 1.0f;
	float m_ScaleZ = 1.0f;
};
