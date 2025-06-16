#pragma once
#include "RenderManager/IRender.h"
#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"
#include "RenderManager/Light/DirectionalLight/DirectionalLightManager.h"

class ISprite : public IRender
{
public:
	ISprite() = default;
	virtual ~ISprite() override = default;

	void EnableLight(bool flag);
	void AddLight(ILightSource* lightSource) const;
	void RemoveLight(ILightSource* lightSource) const;

	void SetVertexShaderPath(const std::wstring& shaderPath);
	void SetPixelShaderPath(const std::wstring& shaderPath);
	void SetTexturePath(const std::string& texturePath);
	void SetOptionalTexturePath(const std::string& texturePath);
	virtual void UpdateTextureResource(const TEXTURE_RESOURCE& resource);

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

protected:
	//~ Common Shader Resource Data
	std::wstring m_VertexShaderPath{ L"Shader/BitmapPlainVS.hlsl" };
	std::wstring m_PixelShaderPath{ L"Shader/BitmapPlainPS.hlsl" };
	std::string m_TexturePath{};
	std::string m_OptionalTexturePath{};

	//~ Light Related Members
	LightManager m_LightManager{};
	inline static bool m_bInitializedStaticBuffer{ false };
	inline static std::unique_ptr<ConstantBuffer<PIXEL_BUFFER_METADATA_GPU>> m_LightMetaCB{ nullptr };
	bool m_LightEnabled{ false };

	//~ World Matrix Constant Buffer
	inline static std::unique_ptr<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>> m_WorldMatrixCB{ nullptr };
};
