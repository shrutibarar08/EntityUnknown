#include "ShaderResource.h"

#include "ExceptionManager/IException.h"
#include "ExceptionManager/RenderException.h"
#include "Utils/FileSystem/FileSystem.h"
#include "Utils/Logger/Logger.h"


void ShaderResource::SetVertexShaderPath(const std::string& path)
{
	if (!FileSystem::IsPathExists(path))
	{
		LOG_WARNING("The path for vertex shader: " + path + " Doest not exist");
		return;
	}

	m_VertexShaderPath = std::wstring(path.begin(), path.end());
}

void ShaderResource::SetVertexShaderPath(const std::wstring& path)
{
	if (!FileSystem::IsPathExists(path))
	{
		std::string c_path = std::string(path.begin(), path.end());
		LOG_WARNING("The path for vertex shader: " + c_path + " Doest not exist");
		return;
	}

	m_VertexShaderPath = path;
}

void ShaderResource::SetPixelShaderPath(const std::string& path)
{
	if (!FileSystem::IsPathExists(path))
	{
		LOG_WARNING("The path for pixel shader: " + path + " Doest not exist");
		return;
	}

	m_PixelShaderPath = std::wstring(path.begin(), path.end());
}

void ShaderResource::SetPixelShaderPath(const std::wstring& path)
{
	if (!FileSystem::IsPathExists(path))
	{
		std::string c_path = std::string(path.begin(), path.end());
		LOG_WARNING("The path for pixel shader: " + c_path + " Doest not exist");
		return;
	}

	m_PixelShaderPath = path;
}

bool ShaderResource::Initialize(ID3D11Device* device, HWND handle)
{

	if (m_VertexShaderPath.empty())
	{
		LOG_ERROR("Shader Resource called to be initialized before setting vertex shader path");
		THROW("Vertex Shader Path was not set");
	}
	if (m_PixelShaderPath.empty())
	{
		LOG_ERROR("Shader Resource called to be initialized before setting pixel shader path");
		THROW("Pixel Shader Path was not set");
	}

	return InitializeShader(device, handle);
}

void ShaderResource::Shutdown()
{
	ShutdownShader();
	return;
}

bool ShaderResource::Render(ID3D11DeviceContext* context, int indexCount, const SHADER_CONSTANT_BUFFER_DESC* desc)
{
	bool result = SetShaderParameters(context, desc);

	if (!result) return false;

	RenderShader(context, indexCount);

	return true;
}

bool ShaderResource::InitializeShader(ID3D11Device* device, HWND handle)
{
	ID3DBlob* errorMessage;
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	D3D11_BUFFER_DESC constantBufferDesc;

	HRESULT result = D3DCompileFromFile(m_VertexShaderPath.c_str(), nullptr,
		nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);

	THROW_RENDER_EXCEPTION_IF_FAILED(result);

	result = D3DCompileFromFile(m_PixelShaderPath.c_str(), nullptr, nullptr,
		"main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	THROW_RENDER_EXCEPTION_IF_FAILED(result);

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), nullptr, &m_VertexShader);
	THROW_RENDER_EXCEPTION_IF_FAILED(result);

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), nullptr, &m_PixelShader);

	//~ Create Input layout
	D3D11_INPUT_ELEMENT_DESC desc[]
	{
		{
			"POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u,
			D3D11_INPUT_PER_VERTEX_DATA, 0u
		},
		{
			"COLOR", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA, 0u
		},
	};
	UINT inputSize = ARRAYSIZE(desc);
	result = device->CreateInputLayout(desc,
		inputSize,
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		&m_Layout);
	THROW_RENDER_EXCEPTION_IF_FAILED(result);

	vertexShaderBuffer->Release();
	pixelShaderBuffer->Release();

	//~ Create Constant Buffer for vertex
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0u;
	constantBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&constantBufferDesc, nullptr, &m_VertexConstantBuffer);
	THROW_RENDER_EXCEPTION_IF_FAILED(result);

	return true;
}

void ShaderResource::ShutdownShader()
{
	m_Layout->Release();
	m_Layout.Reset();
	m_PixelShader->Release();
	m_PixelShader.Reset();
	m_VertexShader->Release();
	m_VertexShader.Reset();
	m_VertexConstantBuffer->Release();
	m_VertexConstantBuffer.Reset();
}

bool ShaderResource::SetShaderParameters(ID3D11DeviceContext* context, const SHADER_CONSTANT_BUFFER_DESC* desc)
{
	D3D11_MAPPED_SUBRESOURCE mapped;

	//~ Transpose
	auto worldMatrix = DirectX::XMMatrixTranspose(desc->World);
	auto projectionMatrix = DirectX::XMMatrixTranspose(desc->Projection);
	auto viewMatrix = DirectX::XMMatrixTranspose(desc->View);

	HRESULT result = context->Map(m_VertexConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	THROW_RENDER_EXCEPTION_IF_FAILED(result);

	auto dataPtr = static_cast<SHADER_CONSTANT_BUFFER_DESC*>(mapped.pData);
	dataPtr->World = worldMatrix;
	dataPtr->Projection = projectionMatrix;
	dataPtr->View = viewMatrix;

	context->Unmap(m_VertexConstantBuffer.Get(), 0u);
	unsigned int bufferNumber = 0;
	context->VSSetConstantBuffers(bufferNumber, 1u, m_VertexConstantBuffer.GetAddressOf());
}

void ShaderResource::RenderShader(ID3D11DeviceContext* context, int indexCount)
{
	context->IASetInputLayout(m_Layout.Get());
	context->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	context->PSSetShader(m_PixelShader.Get(), nullptr, 0u);

	context->DrawIndexed(indexCount, 0, 0);
}
