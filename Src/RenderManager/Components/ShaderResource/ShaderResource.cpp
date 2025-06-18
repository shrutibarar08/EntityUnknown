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
		LOG_WARNING("Texture FILE PATH DOES NOT EXIT - " + path);
	}
	m_TexturePath = path;
}

void ShaderResource::SetTexture(const TEXTURE_RESOURCE& textureResource)
{
	m_TextureResource = textureResource;
}

void ShaderResource::SetTextureSlot(int slot)
{
	m_TextureShader_Slot = slot;
}

void ShaderResource::UpdateTextureResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Texture Resources with uninitialized resources");
	}
	m_TextureResource = textureResource;
}

void ShaderResource::SetSecondaryTexture(const std::string& path)
{
	if (!FileSystem::IsPathExists(path))
	{
		LOG_WARNING("Secondary Texture FILE PATH DOES NOT EXIT - " + path);
	}
	m_SecondaryTexturePath = path;
}

void ShaderResource::SetSecondaryTexture(const TEXTURE_RESOURCE& textureResource)
{
	m_TextureResource = textureResource;
}

void ShaderResource::SetLightMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Normal Map FILE PATH DOES NOT EXIT - " + mapPath);
	}
	m_LightMapPath = mapPath;
}

void ShaderResource::SetLightMap(const TEXTURE_RESOURCE& textureResource)
{
	m_LightMapResource = textureResource;
}

void ShaderResource::SetLightMapSlot(int slot)
{
	m_LightMap_Slot = slot;
}

void ShaderResource::UpdateLightMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Normal Map Resources with uninitialized resources");
	}
	m_LightMapResource = textureResource;
}

void ShaderResource::SetAlphaMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Alpha Map FILE PATH DOES NOT EXIT - " + mapPath);
	}
	m_AlphaMapPath = mapPath;
}

void ShaderResource::SetAlphaMap(const TEXTURE_RESOURCE& textureResource)
{
	m_AlphaMapResource = textureResource;
}

void ShaderResource::SetAlphaMapSlot(int slot)
{
	m_AlphaMapping_Slot = slot;
}

void ShaderResource::UpdateAlphaMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Alpha Map Resources with uninitialized resources");
	}
	m_AlphaMapResource = textureResource;
}

float ShaderResource::GetAlphaValue() const
{
	return m_AlphaValue;
}

void ShaderResource::SetAlphaValue(float value)
{
	m_AlphaValue = value;
}

void ShaderResource::SetNormalMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Normal Map FILE PATH DOES NOT EXIT - " + mapPath);
	}
	m_NormalMapPath = mapPath;
}

void ShaderResource::SetNormalMap(const TEXTURE_RESOURCE& textureResource)
{
	m_NormalMapResource = textureResource;
}

void ShaderResource::SetNormalMapSlot(int slot)
{
	m_NormalMap_Slot = slot;
}

void ShaderResource::UpdateNormalMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Normal Map Resources with uninitialized resources");
	}
	m_NormalMapResource = textureResource;
}

void ShaderResource::SetHeightMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Height Map FILE PATH DOES NOT EXIST - " + mapPath);
	}
	m_HeightMapPath = mapPath;
}

void ShaderResource::SetHeightMap(const TEXTURE_RESOURCE& textureResource)
{
	m_HeightMapResource = textureResource;
}

void ShaderResource::SetHeightMapSlot(int slot)
{
	m_HeightMap_Slot = slot;
}

void ShaderResource::UpdateHeightMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Height Map Resources with uninitialized resources");
	}
	m_HeightMapResource = textureResource;
}

void ShaderResource::SetRoughnessMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Roughness Map FILE PATH DOES NOT EXIST - " + mapPath);
	}
	m_RoughnessMapPath = mapPath;
}

void ShaderResource::SetRoughnessMap(const TEXTURE_RESOURCE& textureResource)
{
	m_RoughnessMapResource = textureResource;
}

void ShaderResource::SetRoughnessMapSlot(int slot)
{
	m_RoughnessMap_Slot = slot;
}

void ShaderResource::UpdateRoughnessMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Roughness Map Resources with uninitialized resources");
	}
	m_RoughnessMapResource = textureResource;
}

void ShaderResource::SetMetalnessMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Metalness Map FILE PATH DOES NOT EXIST - " + mapPath);
	}
	m_MetalnessMapPath = mapPath;
}

void ShaderResource::SetMetalnessMap(const TEXTURE_RESOURCE& textureResource)
{
	m_MetalnessMapResource = textureResource;
}

void ShaderResource::SetMetalnessMapSlot(int slot)
{
	m_MetalnessMap_Slot = slot;
}

void ShaderResource::UpdateMetalnessMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Metalness Map Resources with uninitialized resources");
	}
	m_MetalnessMapResource = textureResource;
}

void ShaderResource::SetAOMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("AO Map FILE PATH DOES NOT EXIST - " + mapPath);
	}
	m_AOMapPath = mapPath;
}

void ShaderResource::SetAOMap(const TEXTURE_RESOURCE& textureResource)
{
	m_AOMapResource = textureResource;
}

void ShaderResource::SetAOMapSlot(int slot)
{
	m_AOMap_Slot = slot;
}

void ShaderResource::UpdateAOMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating AO Map Resources with uninitialized resources");
	}
	m_AOMapResource = textureResource;
}

void ShaderResource::SetSpecularMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Specular Map FILE PATH DOES NOT EXIST - " + mapPath);
	}
	m_SpecularMapPath = mapPath;
}

void ShaderResource::SetSpecularMap(const TEXTURE_RESOURCE& textureResource)
{
	m_SpecularMapResource = textureResource;
}

void ShaderResource::SetSpecularMapSlot(int slot)
{
	m_SpecularMap_Slot = slot;
}

void ShaderResource::UpdateSpecularMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Specular Map Resources with uninitialized resources");
	}
	m_SpecularMapResource = textureResource;
}

void ShaderResource::SetEmissiveMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Emissive Map FILE PATH DOES NOT EXIST - " + mapPath);
	}
	m_EmissiveMapPath = mapPath;
}

void ShaderResource::SetEmissiveMap(const TEXTURE_RESOURCE& textureResource)
{
	m_EmissiveMapResource = textureResource;
}

void ShaderResource::SetEmissiveMapSlot(int slot)
{
	m_EmissiveMap_Slot = slot;
}

void ShaderResource::UpdateEmissiveMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Emissive Map Resources with uninitialized resources");
	}
	m_EmissiveMapResource = textureResource;
}

void ShaderResource::SetDisplacementMap(const std::string& mapPath)
{
	if (!FileSystem::IsPathExists(mapPath))
	{
		LOG_WARNING("Displacement Map FILE PATH DOES NOT EXIST - " + mapPath);
	}
	m_DisplacementMapPath = mapPath;
}

void ShaderResource::SetDisplacementMap(const TEXTURE_RESOURCE& textureResource)
{
	m_DisplacementMapResource = textureResource;
}

void ShaderResource::SetDisplacementMapSlot(int slot)
{
	m_DisplacementMap_Slot = slot;
}

void ShaderResource::UpdateDisplacementMapResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Displacement Map Resources with uninitialized resources");
	}
	m_DisplacementMapResource = textureResource;
}

void ShaderResource::SetSecondaryTextureSlot(int slot)
{
	m_SecondaryTextureShader_Slot = slot;
}

void ShaderResource::UpdateSecondaryTextureResource(const TEXTURE_RESOURCE& textureResource)
{
	if (!textureResource.IsInitialized())
	{
		LOG_WARNING("Shader Resource - Updating Secondary Texture Resources with uninitialized resources");
	}
	m_SecondaryTextureResource = textureResource;
}

bool ShaderResource::IsSecondaryTextureInitialized() const
{
	return m_SecondaryTextureResource.IsInitialized();
}

bool ShaderResource::IsLightMapInitialized() const
{
	return m_LightMapResource.IsInitialized();
}

bool ShaderResource::IsAlphaMapInitialized() const
{
	return m_AlphaMapResource.IsInitialized();
}

bool ShaderResource::IsNormalMapInitialized() const
{
	return m_NormalMapResource.IsInitialized();
}

bool ShaderResource::IsHeightMapInitialized() const
{
	return m_HeightMapResource.IsInitialized();
}

bool ShaderResource::IsRoughnessMapInitialized() const
{
	return m_RoughnessMapResource.IsInitialized();
}

bool ShaderResource::IsMetalnessMapInitialized() const
{
	return m_MetalnessMapResource.IsInitialized();
}

bool ShaderResource::IsAOMapInitialized() const
{
	return m_AOMapResource.IsInitialized();
}

bool ShaderResource::IsSpecularMapInitialized() const
{
	return m_SpecularMapResource.IsInitialized();
}

bool ShaderResource::IsEmissiveMapInitialized() const
{
	return m_EmissiveMapResource.IsInitialized();
}

bool ShaderResource::IsDisplacementMapInitialized() const
{
	return m_DisplacementMapResource.IsInitialized();
}

bool ShaderResource::Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
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

	if (!BuildTexture(device, deviceContext, m_TexturePath, m_TextureResource))
	{
		LOG_WARNING("ShaderResource::Build - Failed to build Texture");
	}

	if (!BuildTexture(device, deviceContext, m_SecondaryTexturePath, m_SecondaryTextureResource))
	{
		if (m_SecondaryTexturePath.empty()) LOG_WARNING("ShaderResource::Build - No Secondary Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Secondary Texture: " + m_SecondaryTexturePath);
	}

	if (!BuildTexture(device, deviceContext, m_LightMapPath, m_LightMapResource))
	{
		if (m_LightMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Normal Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Normal Map: " + m_LightMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_AlphaMapPath, m_AlphaMapResource))
	{
		if (m_AlphaMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Alpha Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Alpha Map: " + m_AlphaMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_NormalMapPath, m_NormalMapResource))
	{
		if (m_NormalMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Normal Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Normal Map: " + m_NormalMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_HeightMapPath, m_HeightMapResource))
	{
		if (m_HeightMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Height Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Height Map: " + m_HeightMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_RoughnessMapPath, m_RoughnessMapResource))
	{
		if (m_RoughnessMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Roughness Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Roughness Map: " + m_RoughnessMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_MetalnessMapPath, m_MetalnessMapResource))
	{
		if (m_MetalnessMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Metalness Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Metalness Map: " + m_MetalnessMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_AOMapPath, m_AOMapResource))
	{
		if (m_AOMapPath.empty()) LOG_WARNING("ShaderResource::Build - No AO Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load AO Map: " + m_AOMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_SpecularMapPath, m_SpecularMapResource))
	{
		if (m_SpecularMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Specular Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Specular Map: " + m_SpecularMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_EmissiveMapPath, m_EmissiveMapResource))
	{
		if (m_EmissiveMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Emissive Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Emissive Map: " + m_EmissiveMapPath);
	}

	if (!BuildTexture(device, deviceContext, m_DisplacementMapPath, m_DisplacementMapResource))
	{
		if (m_DisplacementMapPath.empty()) LOG_WARNING("ShaderResource::Build - No Displacement Map Texture Given");
		else LOG_WARNING("ShaderResource::Build - Failed to Load Displacement Map: " + m_DisplacementMapPath);
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
	context->PSSetSamplers(0u, 1u, m_Sampler.GetAddressOf());

	if (m_TextureResource.IsInitialized())
	{
		ID3D11ShaderResourceView* resources[]{ m_TextureResource.ShaderResourceView };
		context->PSSetShaderResources(m_TextureShader_Slot, 1, resources);
	}
	if (m_SecondaryTextureResource.IsInitialized())
	{
		ID3D11ShaderResourceView* optResources[]{ m_SecondaryTextureResource.ShaderResourceView };
		context->PSSetShaderResources(m_SecondaryTextureShader_Slot, 1, optResources);

	}
	if (m_LightMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* normalResources[]{ m_LightMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_LightMap_Slot, 1, normalResources);
	}
	if (m_AlphaMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* alphaResources[]{ m_AlphaMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_AlphaMapping_Slot, 1, alphaResources);
	}
	if (m_NormalMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* normalResources[]{ m_NormalMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_NormalMap_Slot, 1, normalResources);
	}
	if (m_HeightMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* heightResources[]{ m_HeightMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_HeightMap_Slot, 1, heightResources);
	}

	if (m_RoughnessMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* roughnessResources[]{ m_RoughnessMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_RoughnessMap_Slot, 1, roughnessResources);
	}

	if (m_MetalnessMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* metalnessResources[]{ m_MetalnessMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_MetalnessMap_Slot, 1, metalnessResources);
	}

	if (m_AOMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* aoResources[]{ m_AOMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_AOMap_Slot, 1, aoResources);
	}

	if (m_SpecularMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* specularResources[]{ m_SpecularMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_SpecularMap_Slot, 1, specularResources);
	}

	if (m_EmissiveMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* emissiveResources[]{ m_EmissiveMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_EmissiveMap_Slot, 1, emissiveResources);
	}

	if (m_DisplacementMapResource.IsInitialized())
	{
		ID3D11ShaderResourceView* displacementResources[]{ m_DisplacementMapResource.ShaderResourceView };
		context->PSSetShaderResources(m_DisplacementMap_Slot, 1, displacementResources);
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

bool ShaderResource::BuildTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& path, TEXTURE_RESOURCE& bindResource)
{
	if (path.empty()) return false;
	bindResource = TextureLoader::GetTexture(device, deviceContext, path);
	return bindResource.IsInitialized();
}
