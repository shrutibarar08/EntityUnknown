#pragma once
#include <DirectXMath.h>

#include "RenderManager/IRender.h"
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "RenderManager/Components/ShaderResource/LightBuffer/LightBuffer.h"
#include "RenderManager/Light/DirectionalLight.h"


class IBitmap final: public IRender
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
	};
	using BitMapBuffer = DynamicModelBufferSource<Vertex2D, uint32_t>;
	using DirectionalBufferConfig = LightBuffer<DIRECTIONAL_Light_DATA, 10, false>;
public:
	IBitmap()					  = default;
	virtual ~IBitmap() override   = default;

	bool IsInitialized() const override;

	void SetVertexShader(const std::wstring& vertexShaderPath);
	void SetPixelShader(const std::wstring& pixelShaderPath);

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

	void SetTexture(const std::string& texture);
	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;

	void AddLight(ILightAnyType* lightSource);
	void RemoveLight(ILightAnyType* lightSource);

private:
	void UpdateVertexBuffer(ID3D11DeviceContext* deviceContext);

private:
	//~ Light Buffer related
	std::wstring m_VertexShaderPath{ L"Shader/Bitmap/BitmapLightVS.hlsl" };
	std::wstring m_PixelShaderPath{ L"Shader/Bitmap/BitmapLightPS.hlsl" };

	LightBufferManager m_LightBufferManager{};
	inline static bool m_LightMetaUpdated{ false };
	inline static std::unique_ptr<ConstantBuffer<PIXEL_LIGHT_META_GPU>> m_LightMetaCB{ nullptr };

	//~ Per Instance Shader Data (still using cache tho hehe)
	std::unique_ptr<ShaderResource> m_ShaderResources{ nullptr };
	std::string m_TextureToAdd{};
	std::shared_ptr<BitMapBuffer> m_SharedBitMapBuffer{ nullptr };
	std::unique_ptr<BitmapInstance<BitMapBuffer>> m_BitMapBuffer{ nullptr };
	bool m_LocalInitialized{ false };

	//~ World Matrix Constant Buffer (1 for all instances)
	inline static bool m_bStaticInitialized{ false };
	inline static std::unique_ptr<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>> m_BitmapWorldMatrixCB{ nullptr };

	Vertex2D m_Vertices[6]{};
	uint32_t m_Indices[6]{};

	int m_LastHeight{ -1 }, m_LastWidth{ -1 };
	int m_LazyWait{ 100 };
};
