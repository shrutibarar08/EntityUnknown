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

	if (light->IsShadowDSVAssigned()) return;
	for (int i = 0; i < m_MaxBufferSize; i++)
	{
		if (!m_AssignedIndex[i])
		{
			light->SetShadowDSV(m_ShadowMapDSVs[i].Get());
			m_AssignedIndex[i] = true;
			m_LightToIndex[light->GetAssignedID()] = i;
			LOG_INFO("Assigned Slice: " + std::to_string(i) + ", On: " + light->GetLightTypeToString() + " with ID: " + std::to_string(light->GetAssignedID()));
			break;
		}
	}
}

void DirectionalLightManager::RemoveLight(ID id)
{
	auto it = m_Lights.find(id);
	if (it == m_Lights.end())
		return;

	DirectionalLight* light = it->second;

	auto itSlice = m_LightToIndex.find(id);
	if (light && light->IsShadowDSVAssigned() && itSlice != m_LightToIndex.end())
	{
		const int slice = itSlice->second;
		m_AssignedIndex[slice] = false;
		m_LightToIndex.erase(itSlice);
	}

	m_Lights.erase(it);
}

void DirectionalLightManager::Clear()
{
	for (auto& id: m_Lights | std::views::keys)
	{
		if (m_Lights[id]->IsShadowDSVAssigned())
		{
			int slice = m_LightToIndex[id];
			m_AssignedIndex[slice] = false;
		}
	}

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

	BuildShadowResources(device);
}

void DirectionalLightManager::Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition)
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

		light->ComputeViewMatrix(ownerPosition);

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
	context->PSSetShaderResources(m_ShadowMap_Slot, 1u, m_ShadowMapSRV.GetAddressOf());
}

void DirectionalLightManager::UnBind(ID3D11DeviceContext* context) const
{
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->PSSetShaderResources(m_Slot, 1u, nullSRV);
	context->PSSetShaderResources(m_ShadowMap_Slot, 1u, nullSRV);
}

void DirectionalLightManager::BuildShadowResources(ID3D11Device* device) const
{
	if (m_bShadowTextureInitialized) return;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = m_ShadowMapSize;
	texDesc.Height = m_ShadowMapSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = m_MaxBufferSize;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	THROW_RENDER_EXCEPTION_IF_FAILED(
		device->CreateTexture2D(&texDesc, nullptr, &m_ShadowMapArray)
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = m_MaxBufferSize;
	srvDesc.Texture2DArray.MipLevels = 1;

	THROW_RENDER_EXCEPTION_IF_FAILED(
		device->CreateShaderResourceView(m_ShadowMapArray.Get(), &srvDesc, &m_ShadowMapSRV)
	);

	m_ShadowMapDSVs.resize(m_MaxBufferSize);
	for (UINT i = 0; i < m_MaxBufferSize; ++i)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Texture2DArray.ArraySize = 1;
		dsvDesc.Texture2DArray.FirstArraySlice = i;

		THROW_RENDER_EXCEPTION_IF_FAILED(
			device->CreateDepthStencilView(m_ShadowMapArray.Get(), &dsvDesc, &m_ShadowMapDSVs[i])
		);
	}

	LOG_INFO("Directional Light Buffer Initialized!");
	m_bShadowTextureInitialized = true;
}
