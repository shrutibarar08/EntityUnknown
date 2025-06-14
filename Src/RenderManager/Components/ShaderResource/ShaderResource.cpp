#include "ShaderResource.h"

#include "ExceptionManager/IException.h"
#include "ExceptionManager/RenderException.h"
#include "Utils/FileSystem/FileSystem.h"
#include "Utils/Logger/Logger.h"

#include <DirectXMath.h>

#include "Blob/BlobBuilder.h"
#include "PixelShader/PixelShader.h"
#include "VertexShader/VertexShader.h"


void ShaderResource::SetVertexShaderPath(const BLOB_BUILDER_DESC& desc)
{
	if (!FileSystem::IsPathExists(desc.FilePath))
	{
		std::string path = std::string(desc.FilePath.begin(), desc.FilePath.end());
		LOG_WARNING("FILE PATH DOES NOT EXIT - " + path);
	}

	m_VertexShaderPath = desc;
}

void ShaderResource::SetPixelShaderPath(const BLOB_BUILDER_DESC& desc)
{
	if (!FileSystem::IsPathExists(desc.FilePath))
	{
		std::string path = std::string(desc.FilePath.begin(), desc.FilePath.end());
		LOG_WARNING("FILE PATH DOES NOT EXIT - " + path);
	}

	m_PixelShaderPath = desc;
}

void ShaderResource::AddElement(
	const std::string& semantic,
	DXGI_FORMAT format, UINT index,
	UINT slot, UINT offset,
	D3D11_INPUT_CLASSIFICATION classification,
	UINT instanceRate)
{
	VertexLayoutElement element
	{
		semantic,
		index,
		format,
		slot,
		offset,
		classification,
		instanceRate
	};

	m_Elements.push_back(element);
}

void ShaderResource::SetTexture(const std::string& path)
{
	if (!FileSystem::IsPathExists(path))
	{
		LOG_WARNING("FILE PATH DOES NOT EXIT - " + path);
	}
	m_TexturePath = path;
}

void ShaderResource::SetTexture(const TEXTURE_RESOURCE& texture)
{
	m_TextureResource = texture;
}

bool ShaderResource::Build(ID3D11Device* device)
{
	if (!BuildVertexShader(device))
	{
		LOG_ERROR("ShaderResource::Build - Failed to build Vertex Shader");
		return false;
	}

	if (!BuildPixelShader(device)) 
	{
		LOG_ERROR("ShaderResource::Build - Failed to build Pixel Shader");
		return false;
	}

	if (!BuildInputLayout(device)) 
	{
		LOG_ERROR("ShaderResource::Build - Failed to build Input Layout");
		return false;
	}

	if (!BuildSampler(device)) 
	{
		LOG_ERROR("ShaderResource::Build - Failed to build Sampler State");
		return false;
	}

	if (!BuildTexture(device))
	{
		LOG_ERROR("ShaderResource::Build - Failed to build Texture");
		return false;
	}

	LOG_INFO("ShaderResource::Build - Successfully built all shader components");
	return true;
}

void ShaderResource::Shutdown()
{
	m_Layout->Release();
	m_Layout.Reset();
	m_PixelShader->Release();
	m_PixelShader.Reset();
	m_VertexShader->Release();
	m_VertexShader.Reset();
}

bool ShaderResource::Render(ID3D11DeviceContext* context) const
{
	if (m_VertexShader == nullptr) THROW("Vertex Shader is null!");
	if (m_Layout == nullptr) THROW("Input layout is null!");
	if (m_PixelShader == nullptr) THROW("Pixel Shader is null!");

	context->IASetInputLayout(m_Layout.Get());
	context->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	context->PSSetShader(m_PixelShader.Get(), nullptr, 0u);

	if (m_TextureResource.IsInitialized())
	{
		assert(m_TextureResource.ShaderResourceView && "SVR is Null!");
		context->PSSetSamplers(0u, 1u, m_Sampler.GetAddressOf());
		ID3D11ShaderResourceView* resources[]{ m_TextureResource.ShaderResourceView };
		context->PSSetShaderResources(1, 1, resources);
	}
	return true;
}

bool ShaderResource::IsTextureInitialized() const
{
	return m_TextureResource.IsInitialized();
}

TEXTURE_RESOURCE ShaderResource::GetTextureResource() const
{
	return m_TextureResource;
}

bool ShaderResource::BuildVertexShader(ID3D11Device* device)
{
	if (m_VertexShaderPath.IsEmpty())
	{
		THROW("Calling Vertex Shader Build Without Providing any initialization desc");
	}
	m_VertexBlob = BlobBuilder::GetBlob(&m_VertexShaderPath);
	m_VertexShader = VertexShader::Get(device, &m_VertexShaderPath);
	return true;
}

bool ShaderResource::BuildPixelShader(ID3D11Device* device)
{
	if (m_PixelShaderPath.IsEmpty())
	{
		THROW("Calling Pixel Shader Build Without Providing any initialization desc");
	}
	m_PixelShader = PixelShader::Get(device, &m_PixelShaderPath);
	return true;
}

bool ShaderResource::BuildInputLayout(ID3D11Device* device)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc;

	for (const auto& element: m_Elements)
	{
		D3D11_INPUT_ELEMENT_DESC desc{};
		desc.SemanticName = element.SemanticName.c_str();
		desc.SemanticIndex = element.SemanticIndex;
		desc.Format = element.Format;
		desc.InputSlot = element.InputSlot;
		desc.AlignedByteOffset = element.AlignedByteOffset;
		desc.InputSlotClass = element.InputSlotClass;
		desc.InstanceDataStepRate = element.InstanceDataStepRate;
		inputDesc.push_back(desc);
	}

	const HRESULT hr = device->CreateInputLayout(
		inputDesc.data(),
		static_cast<UINT>(inputDesc.size()),
		m_VertexBlob->GetBufferPointer(),
		m_VertexBlob->GetBufferSize(),
		&m_Layout
	);

	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	return true;
}

bool ShaderResource::BuildSampler(ID3D11Device* device)
{
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	const HRESULT hr = device->CreateSamplerState(&samplerDesc, &m_Sampler);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	return true;
}

bool ShaderResource::BuildTexture(ID3D11Device* device)
{
	if (m_TexturePath.empty()) return false;
	m_TextureResource = TextureLoader::GetTexture(device, m_TexturePath);
	return true;
}
