#pragma once
#include "RenderManager/IRender.h"
#include "RenderManager/Components/ShaderResource/LightBuffer/LightBuffer.h"
#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"
#include "RenderManager/Light/DirectionalLight.h"

class ISprite : public IRender
{
	using DirectionalBufferConfig = LightBuffer<DIRECTIONAL_Light_DATA, 10, false>;

public:
	ISprite() = default;
	virtual ~ISprite() override = default;

	void EnableLight(bool flag);
	void AddLight(ILightAnyType* lightSource);
	void RemoveLight(ILightAnyType* lightSource);

	void SetVertexShaderPath(const std::wstring& shaderPath);
	void SetPixelShaderPath(const std::wstring& shaderPath);
	void SetTexturePath(const std::string& texturePath);
	virtual void UpdateTextureResource(const TEXTURE_RESOURCE& resource) = 0;

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

protected:
	//~ Common Shader Resource Data
	std::wstring m_VertexShaderPath{ L"Shader/BitmapPlainVS.hlsl" };
	std::wstring m_PixelShaderPath{ L"Shader/BitmapPlainPS.hlsl" };
	std::string m_TexturePath{};

	//~ Light Related Members
	LightBufferManager m_LightBufferManager{};
	inline static bool m_bInitializedStaticBuffer{ false };
	inline static std::unique_ptr<ConstantBuffer<PIXEL_LIGHT_META_GPU>> m_LightMetaCB{ nullptr };
	bool m_LightEnabled{ false };

	//~ World Matrix Constant Buffer
	inline static std::unique_ptr<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>> m_WorldMatrixCB{ nullptr };
};
