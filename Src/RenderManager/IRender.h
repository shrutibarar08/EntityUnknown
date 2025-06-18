#pragma once

#include "SystemManager/PrimaryID.h"
#include "Components/ConstantBuffer.h"

#include <DirectXMath.h>
#include <memory>
#include <d3d11.h>

#include "Collision/Cube/CubeCollider.h"
#include "Components/ShaderResource/ShaderResource.h"
#include "Light/LightManager.h"
#include "RigidBody/RigidBody.h"


typedef struct CAMERA_INFORMATION_CPU_DESC
{
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMFLOAT3 CameraPosition;
}CAMERA_INFORMATION_DESC;
	
//~ Must attach to slot_1
typedef struct VERTEX_BUFFER_METADATA_GPU
{
	DirectX::XMMATRIX WorldMatrix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMMATRIX NormalMatrix;
	DirectX::XMFLOAT3   CameraPosition;
	float               Padding;
}VERTEX_BUFFER_METADATA_GPU;

typedef struct PIXEL_BUFFER_METADATA_GPU
{
	int DirectionalLightCount;
	int SpotLightCount;
	int PointLightCount;
	int DebugLine;

	int Texture;
	int MultiTexturing;
	int LightMap;
	int AlphaMap;

	float AlphaValue = 1.0f;
	int NormalMap;
	int HeightMap;
	int RoughnessMap;
	int MetalnessMap;

	int AOMap;
	int SpecularMap;
	int EmissiveMap;
	int DisplacementMap;

	float padding[1]; // Maintain 16-byte alignment (total size = 64 bytes)
} PIXEL_BUFFER_METADATA_GPU;

class IRender: public PrimaryID
{
public:
	IRender();
	virtual ~IRender()					= default;
	IRender(const IRender&)				= delete;
	IRender(IRender&&)					= delete;
	IRender& operator=(const IRender&)	= delete;
	IRender& operator=(IRender&&)		= delete;

	virtual bool Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual bool Render(ID3D11DeviceContext* deviceContext);

	virtual void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) = 0;
	virtual bool IsInitialized() const = 0;

	void SetScreenWidth(int width);
	void SetScreenHeight(int height);
	void SetDirty(bool flag);
	bool IsDirty() const;

	void AddLight(ILightSource* lightSource) const;
	void RemoveLight(ILightSource* lightSource) const;

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

	DirectX::XMMATRIX GetNormalTransform() const;
	ShaderResource* GetShaderResource();
	PIXEL_BUFFER_METADATA_GPU GetPixelCBMetaData() const;

	//~ Helper
	static void PrintMatrix(const DirectX::XMMATRIX& mat);

protected:
	virtual void BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext) = 0;
	void EnableLight(bool flag);

	void UpdateVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const;
	void UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, bool debug=false) const;

	void BindVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const;
	void BindPixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const;

protected:
	//~ Body Specifics
	bool m_bDirty{ false };
	RigidBody m_RigidBody{};
	std::unique_ptr<CubeCollider> m_CubeCollider{ nullptr };

	//~ Light and Shaders
	bool m_LightEnabled{ true };
	LightManager m_LightManager{};
	ShaderResource m_ShaderResources{};

	//~ Meta Constant Buffers
	inline static bool m_bCommonDataInitialized{ false };
	VERTEX_BUFFER_METADATA_GPU m_WorldMatrixGPU{};
	inline static std::unique_ptr<ConstantBuffer<VERTEX_BUFFER_METADATA_GPU>> m_VertexMetadataCB{ nullptr };
	inline static std::unique_ptr<ConstantBuffer<PIXEL_BUFFER_METADATA_GPU>> m_PixelMetadataCB{ nullptr };
	int m_VertexMetadataCB_Slot{ 0 };
	int m_PixelMetadataCB_Slot{ 0 };

	//~ Rendering Infos 
	DirectX::XMFLOAT3 m_Scale{1.f, 1.f, 1.f};
	int m_ScreenWidth{ 1280 };
	int m_ScreenHeight{ 720 };
};
