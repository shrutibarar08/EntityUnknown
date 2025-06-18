#pragma once
#include <d3d11.h>

#include <string>
#include <vector>
#include <wrl/client.h>

#include "Blob/BlobBuilder.h"
#include "TextureResource/TextureLoader.h"


struct VertexLayoutElement
{
	std::string SemanticName;
	UINT SemanticIndex = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
	UINT InputSlot = 0;
	UINT AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	D3D11_INPUT_CLASSIFICATION InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	UINT InstanceDataStepRate = 0;
};

class ShaderResource
{
public:
	ShaderResource() = default;
	virtual ~ShaderResource() = default;

	ShaderResource(const ShaderResource&) = delete;
	ShaderResource(ShaderResource&&) = delete;
	ShaderResource& operator=(const ShaderResource&) = delete;
	ShaderResource& operator=(ShaderResource&&) = delete;

	void SetVertexShaderPath(const BLOB_BUILDER_DESC& desc);
	void SetPixelShaderPath(const BLOB_BUILDER_DESC& desc);

	void AddElement(const std::string& semantic, DXGI_FORMAT format, UINT index = 0,
		UINT slot = 0, UINT offset = D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_CLASSIFICATION classification = D3D11_INPUT_PER_VERTEX_DATA,
		UINT instanceRate = 0);

	void SetTexture(const std::string& path);
	void SetTexture(const TEXTURE_RESOURCE& textureResource);
	void SetTextureSlot(int slot);
	void UpdateTextureResource(const TEXTURE_RESOURCE& textureResource);

	void SetSecondaryTexture(const std::string& path);
	void SetSecondaryTexture(const TEXTURE_RESOURCE& textureResource);
	void SetSecondaryTextureSlot(int slot);
	void UpdateSecondaryTextureResource(const TEXTURE_RESOURCE& textureResource);

	void SetLightMap(const std::string& mapPath);
	void SetLightMap(const TEXTURE_RESOURCE& textureResource);
	void SetLightMapSlot(int slot);
	void UpdateLightMapResource(const TEXTURE_RESOURCE& textureResource);

	void SetAlphaMap(const std::string& mapPath);
	void SetAlphaMap(const TEXTURE_RESOURCE& textureResource);
	void SetAlphaMapSlot(int slot);
	void UpdateAlphaMapResource(const TEXTURE_RESOURCE& textureResource);
	float GetAlphaValue() const;
	void SetAlphaValue(float value);

	void SetNormalMap(const std::string& mapPath);
	void SetNormalMap(const TEXTURE_RESOURCE& textureResource);
	void SetNormalMapSlot(int slot);
	void UpdateNormalMapResource(const TEXTURE_RESOURCE& textureResource);

	bool Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context) const;
	TEXTURE_RESOURCE GetTextureResource() const;

	bool IsTextureInitialized() const;
	bool IsSecondaryTextureInitialized() const;
	bool IsLightMapInitialized() const;
	bool IsAlphaMapInitialized() const;
	bool IsNormalMapInitialized() const;

private:
	bool BuildVertexShader(ID3D11Device* device);
	bool BuildPixelShader(ID3D11Device* device);
	bool BuildInputLayout(ID3D11Device* device);
	bool BuildSampler(ID3D11Device* device);
	static bool BuildTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& path, TEXTURE_RESOURCE& bindResource);

private:
	//~ Shaders
	BLOB_BUILDER_DESC m_VertexShaderPath;
	BLOB_BUILDER_DESC m_PixelShaderPath;
	std::vector<VertexLayoutElement> m_Elements;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_Layout{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_Sampler{ nullptr };
	ID3DBlob* m_VertexBlob{ nullptr };

	//~ Main Texture
	int m_TextureShader_Slot{ 3 };
	std::string m_TexturePath;
	TEXTURE_RESOURCE m_TextureResource{};

	//~ Secondary Texture
	int m_SecondaryTextureShader_Slot{ 4 };
	std::string m_SecondaryTexturePath;
	TEXTURE_RESOURCE m_SecondaryTextureResource{};

	//~ Light Map
	int m_LightMap_Slot{ 5 };
	std::string m_LightMapPath;
	TEXTURE_RESOURCE m_LightMapResource{};

	//~ Alpha Mapping
	int m_AlphaMapping_Slot{ 6 };
	std::string m_AlphaMapPath;
	TEXTURE_RESOURCE m_AlphaMapResource{};
	float m_AlphaValue{ 1.0f };

	//~ Normal Mapping
	int m_NormalMap_Slot{ 7 };
	std::string m_NormalMapPath;
	TEXTURE_RESOURCE m_NormalMapResource{};
};
 