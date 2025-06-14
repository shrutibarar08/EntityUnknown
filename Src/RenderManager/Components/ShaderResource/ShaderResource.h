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
	void SetTexture(const TEXTURE_RESOURCE& texture);

	bool Build(ID3D11Device* device);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context) const;

	TEXTURE_RESOURCE GetTextureResource() const;

private:
	bool BuildVertexShader(ID3D11Device* device);
	bool BuildPixelShader(ID3D11Device* device);
	bool BuildInputLayout(ID3D11Device* device);
	bool BuildSampler(ID3D11Device* device);
	bool BuildTexture(ID3D11Device* device);

private:
	BLOB_BUILDER_DESC m_VertexShaderPath;
	BLOB_BUILDER_DESC m_PixelShaderPath;
	std::string m_TexturePath;
	std::vector<VertexLayoutElement> m_Elements;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader{ nullptr };
	ID3DBlob* m_VertexBlob{ nullptr };

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_Layout{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_Sampler{ nullptr };
	TEXTURE_RESOURCE m_TextureResource{};
};
