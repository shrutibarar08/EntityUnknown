#pragma once
#include "RenderManager/Components/ConstantBuffer.h"
#include "RenderManager/Components/ShaderResource/LightBuffer/LightBuffer.h"
#include "RenderManager/Light/DirectionalLight.h"
#include "SystemManager/PrimaryID.h"


typedef struct CAMERA_MATRIX_DESC
{
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
}CAMERA_MATRIX_DESC;

typedef struct CAMERA_BUFFER
{
	DirectX::XMFLOAT3 CameraPosition;
	float padding;
}CAMERA_BUFFER;

typedef struct WORLD_TRANSFORM
{
	DirectX::XMMATRIX WorldMatrix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMMATRIX NormalMatrix;
}WORLD_TRANSFORM;

class IModel: public PrimaryID
{
	using DirectionalBufferConfig = LightBuffer<DIRECTIONAL_Light_DATA, 10, false>;
public:
	IModel() = default;
	virtual ~IModel() = default;

	virtual bool IsInitialized() const = 0;

	bool Build(ID3D11Device* device);;
	bool Render(ID3D11DeviceContext* deviceContext);
	void UpdateTransformation(const CAMERA_MATRIX_DESC* matrix);
	void UpdateCameraBuffer(const CAMERA_BUFFER& cameraBuffer);

	WORLD_TRANSFORM GetWorldTransform() const;
	DirectX::XMVECTOR GetTranslationVector() const;
	DirectX::XMVECTOR GetRotationVector() const;
	DirectX::XMVECTOR GetScaleVector() const;
	DirectX::XMMATRIX GetTransform() const;
	DirectX::XMMATRIX GetNormalTransform() const;

	void AddLight(ILightDataBase* lightSource);
	void RemoveLight(ILightDataBase* lightSource);

	// --- Scale Set/Add ---
	void SetScale(float x, float y, float z);
	void SetScaleX(float x);
	void SetScaleY(float y);
	void SetScaleZ(float z);

	void AddScale(float x, float y, float z);
	void AddScaleX(float x);
	void AddScaleY(float y);
	void AddScaleZ(float z);

	// --- Rotation Set/Add ---
	void SetRotation(float pitch, float yaw, float roll);
	void SetPitch(float pitch);
	void SetYaw(float yaw);
	void SetRoll(float roll);

	void AddPitch(float delta);
	void AddYaw(float delta);
	void AddRoll(float delta);

	// --- Translation Set/Add ---
	void SetTranslation(float x, float y, float z);
	void SetTranslationX(float x);
	void SetTranslationY(float y);
	void SetTranslationZ(float z);

	void AddTranslation(float x, float y, float z);
	void AddTranslationX(float x);
	void AddTranslationY(float y);
	void AddTranslationZ(float z);

protected:
	virtual bool BuildChild(ID3D11Device* device) = 0;
	virtual bool RenderChild(ID3D11DeviceContext* deviceContext) = 0;

protected:
	//~ Light Buffer
	LightBufferManager m_LightBufferManager{};

	struct alignas(16) LightMeta
	{
		int gDirectionalLightCount = 0;
		float padding[3] = {};
	};
	inline static bool m_LightMetaUpdated{ false };
	inline static std::unique_ptr<ConstantBuffer<LightMeta>> m_LightMetaCB{ nullptr };

	//~ Buffer Gonna be same so static should work just like a charm hehe
	CAMERA_BUFFER m_CameraBufferData{};
	inline static std::unique_ptr<IConstantBuffer> m_CameraBuffer{ nullptr };

	WORLD_TRANSFORM m_WorldTransform{};

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
