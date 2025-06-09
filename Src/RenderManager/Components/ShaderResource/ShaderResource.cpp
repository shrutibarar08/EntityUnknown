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

bool ShaderResource::Build(ID3D11Device* device)
{
	if (!BuildVertexShader(device)) return false;
	if (!BuildPixelShader(device)) return false;
	if (!BuildInputLayout(device)) return false;
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
	context->IASetInputLayout(m_Layout.Get());
	context->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	context->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
	return true;
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
