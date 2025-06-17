#include "DirectionalLightManager.h"
#include "ExceptionManager/RenderException.h"

#include <ranges>

DirectionalLightManager::DirectionalLightManager(int maxSize, UINT slot)
	: m_Slot(slot), m_MaxBufferSize(maxSize)
{
}

void DirectionalLightManager::AddLight(DirectionalLight* light)
{
	if (!light) return;

	ID id = light->GetAssignedID();

	if (m_Lights.contains(id))
	{
		return;
	}

	if (m_Lights.size() >= static_cast<size_t>(m_MaxBufferSize))
	{
		return;
	}

	m_Lights[id] = light;
}

void DirectionalLightManager::RemoveLight(ID id)
{
	if (!m_Lights.contains(id))
	{
		return;
	}

	m_Lights.erase(id);
}

void DirectionalLightManager::Clear()
{
	m_Lights.clear();
	m_GPUData.clear();
}

void DirectionalLightManager::Build(ID3D11Device* device)
{
	static_assert(sizeof(DIRECTIONAL_LIGHT_GPU_DATA) % 16 == 0, "DIRECTIONAL_LIGHT_GPU_DATA must be 16-byte aligned.");

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(DIRECTIONAL_LIGHT_GPU_DATA) * m_MaxBufferSize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.StructureByteStride = sizeof(DIRECTIONAL_LIGHT_GPU_DATA);
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_Buffer);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_MaxBufferSize;

	hr = device->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, &m_SRV);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);
}

void DirectionalLightManager::Update(ID3D11DeviceContext* context)
{
	m_GPUData.clear();
	m_GPUData.reserve(m_MaxBufferSize);

	int count = 0;
	for (auto it = m_Lights.begin(); it != m_Lights.end(); )
	{
		DirectionalLight* light = it->second;

		if (!light)
		{
			it = m_Lights.erase(it);
			continue;
		}

		if (count >= m_MaxBufferSize)
		{
			++it;
			continue;
		}

		m_GPUData.push_back(light->GetLightData());
		++count;
		++it;
	}

	while (m_GPUData.size() < m_MaxBufferSize) m_GPUData.emplace_back(DIRECTIONAL_LIGHT_GPU_DATA{});

	context->UpdateSubresource(
		m_Buffer.Get(),
		0,
		nullptr,
		m_GPUData.data(),
		0,
		0
	);
}

void DirectionalLightManager::Bind(ID3D11DeviceContext* context) const
{
	context->PSSetShaderResources(m_Slot, 1u, m_SRV.GetAddressOf());
}
