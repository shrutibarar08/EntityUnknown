#pragma once

#include "SystemManager/PrimaryID.h"
#include "RenderManager/Components/ConstantBuffer.h"
#include "Collision/Cube/CubeCollider.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "RenderManager/Light/LightManager.h"

#include <DirectXMath.h>
#include <memory>
#include <d3d11.h>

#include <nlohmann/json.hpp>

#define ROOT_PATH "EntityUnknown"

#include "Imgui/imgui.h"

class LevelEditorContext;

namespace UIHelpers
{
	inline std::unordered_map<ImGuiID, std::array<char, 256>> g_PathBuffers{};
}

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
	DirectX::XMFLOAT3 CameraPosition;
	float             Padding;
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
	virtual bool RenderDepthOnly(
		ID3D11DeviceContext* deviceContext,
		const DirectX::XMMATRIX& lightViewMatrix,
		const DirectX::XMMATRIX& ProjectionMatrix
	);
	virtual bool UnBind(ID3D11DeviceContext* deviceContext);

	virtual void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) = 0;
	virtual bool IsInitialized() const = 0;
	virtual void ResetInitialization() {}

	virtual void RenderControlUI(LevelEditorContext* context);

	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topoloy) { m_PrimitiveTopology = topoloy; }

	void SetTypeName(const std::string& name) { m_TypeName = name; }
	std::string GetTypeName() const { return m_TypeName; }

	void SetScreenWidth(int width);
	void SetScreenHeight(int height);
	void SetDirty(bool flag);
	bool IsDirty() const;

	virtual void		   LoadRenderSaveData(const nlohmann::json& json);
	virtual nlohmann::json GetRenderSaveData() const;

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

	void SetTextureMultiplier(int valueX, int valueY);
	bool IsTransparent() const;
	void SetTransparent(bool flag);

	DirectX::XMMATRIX GetNormalTransform() const;
	ShaderResource* GetShaderResource();
	PIXEL_BUFFER_METADATA_GPU GetPixelCBMetaData() const;

	std::string& GetName() { return m_Name; }
	void SetName(const std::string& name) { m_Name = name; }

	//~ Helper
	static void PrintMatrix(const DirectX::XMMATRIX& mat);
	static std::string OpenFileDialog(const char* filter = "All Files\0*.*\0");

	bool IsDebugOnly() const { return m_bRenderOnDebugOnly; }
	void SetDebugOnly(bool flag) { m_bRenderOnDebugOnly = flag; }

	LightManager& GetLightManager() { return m_LightManager; }

protected:
	virtual void BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext) = 0;
	virtual void RenderGeometry(ID3D11DeviceContext* deviceContext) = 0;
	void EnableLight(bool flag);

	void UpdateVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const;
	void UpdateVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, const VERTEX_BUFFER_METADATA_GPU& gpuData) const;

	void UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, bool debug=false) const;
	void UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, const PIXEL_BUFFER_METADATA_GPU& gpuData) const;

	void BindVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const;
	void BindPixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const;

	//~ Helpers
	virtual void UI_Section_ObjectAndRender(LevelEditorContext* ctx);
	virtual void UI_Section_TransformAndPhysics(LevelEditorContext* ctx);
	virtual void UI_Section_Textures(LevelEditorContext* ctx);

	void UI_PathFieldWithApplyAndDnD(const char* label,
		const std::string& currentValue,
		const std::function<void(const std::string&)>& applySetter,
		bool showPreview = true) const;

	static void UI_SafeCopy(char* dst, size_t dstSize, const std::string& src);
	static int  UI_TopologyToIndex(D3D_PRIMITIVE_TOPOLOGY t);
	static D3D_PRIMITIVE_TOPOLOGY UI_IndexToTopology(int idx);

	virtual const char* UI_ObjectRenameLabel() const { return "Object Name"; }

protected:

	std::string m_TypeName{ "IRender" };
	std::string m_Name{ "Not Given" };
	//~ Body Specifics
	bool m_bTransparent{ false };
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

	int m_TextureMultiplierX{ 1 };
	int m_TextureMultiplierY{ 1 };

	D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology{ D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	bool m_bRenderOnDebugOnly{ false };
};
