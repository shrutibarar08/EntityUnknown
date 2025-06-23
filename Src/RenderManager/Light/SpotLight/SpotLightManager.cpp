#include "SpotLightManager.h"
#include "ExceptionManager/RenderException.h"

#include <algorithm>
#include <ranges>

SpotLightManager::SpotLightManager(int maxSize, UINT slot)
	: m_Slot(slot), m_MaxBufferSize(maxSize)
{
}

void SpotLightManager::AddLight(SpotLight* light)
{
	if (!light) return;

	ID id = light->GetAssignedID();
	if (m_Lights.contains(id)) return;

	m_Lights[id] = light;
	m_Dirty = true;

	if (light->IsShadowDSVAssigned()) return;
	for (int i = 0; i < m_MaxBufferSize; i++)
	{
		if (!m_AssignedIndex[i])
		{
			light->SetShadowDSV(m_ShadowMapDSVs[i].Get());
			m_AssignedIndex[i] = true;
			m_LightToIndex[light->GetAssignedID()] = i;
			LOG_INFO("Assigned Slice: " + std::to_string(i) + ", On: " + light->GetLightName() + " with ID: " + std::to_string(light->GetAssignedID()));
			break;
		}
	}
}

void SpotLightManager::RemoveLight(ID id)
{
	if (!m_Lights.contains(id))
	{
		return;
	}

	if (m_Lights[id]->IsShadowDSVAssigned())
	{
		int slice = m_LightToIndex[id];
		m_LightToIndex.erase(id);
		m_AssignedIndex[slice] = false;
	}

	m_Lights.erase(id);
	m_Dirty = true;
}

void SpotLightManager::Clear()
{
	for (auto& id : m_Lights | std::views::keys)
	{
		if (m_Lights[id]->IsShadowDSVAssigned())
		{
			int slice = m_LightToIndex[id];
			m_AssignedIndex[slice] = false;
		}
	}

	m_Lights.clear();
	m_GPUData.clear();
	m_Dirty = true;
}

void SpotLightManager::Build(ID3D11Device* device)
{
	static_assert(sizeof(SPOT_LIGHT_GPU_DATA) % 16 == 0, "SPOT_LIGHT_GPU_DATA must be 16-byte aligned.");

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(SPOT_LIGHT_GPU_DATA) * m_MaxBufferSize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.StructureByteStride = sizeof(SPOT_LIGHT_GPU_DATA);
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.CPUAccessFlags = 0;


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

void SpotLightManager::Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition)
{
	m_GPUData.clear();
	m_GPUData.reserve(m_MaxBufferSize);

	// Create a vector of light distances
	std::vector<LightDistance> lightDistances;

	// Collect the distances of all available lights
	for (const auto& [id, light] : m_Lights)
	{
		if (!light) continue;

		DirectX::XMFLOAT3 lightPos = light->GetPosition();  // Assuming GetPosition() returns a XMFLOAT3
		float distance = CalculateDistance(ownerPosition, lightPos);

		lightDistances.push_back(LightDistance{ id, distance });
	}

	// Sort lights by distance (closest first)
	std::sort(lightDistances.begin(), lightDistances.end());

	// Push the closest lights into the GPU data, up to the max buffer size
	int count = lightDistances.size() > m_MaxBufferSize? m_MaxBufferSize: lightDistances.size();

	for (int i = 0; i < count; ++i)
	{
		SpotLight* light = m_Lights[lightDistances[i].id];
		light->ComputeViewMatrix(ownerPosition);
		m_GPUData.push_back(light->GetLightData());
	}

	// Pad the rest with empty slots if we don't have enough lights
	while (m_GPUData.size() < m_MaxBufferSize)
	{
		m_GPUData.emplace_back(SPOT_LIGHT_GPU_DATA{});
	}

	// Update the buffer with the light data
	context->UpdateSubresource(
		m_Buffer.Get(),
		0,
		nullptr,
		m_GPUData.data(),
		0,
		0
	);
}

void SpotLightManager::Bind(ID3D11DeviceContext* context) const
{
	context->PSSetShaderResources(m_Slot, 1u, m_SRV.GetAddressOf());
	context->PSSetShaderResources(m_ShadowMap_Slot, 1u, m_ShadowMapSRV.GetAddressOf());
}

void SpotLightManager::UnBind(ID3D11DeviceContext* context) const
{
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->PSSetShaderResources(m_Slot, 1u, nullSRV);
	context->PSSetShaderResources(m_ShadowMap_Slot, 1u, nullSRV);
}

int SpotLightManager::GetLightCount() const
{
	return static_cast<int>(m_Lights.size());
}

float SpotLightManager::CalculateDistance(const DirectX::XMVECTOR& a, const DirectX::XMFLOAT3& b)
{
	const DirectX::XMVECTOR bVec = DirectX::XMLoadFloat3(&b);
	return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(a, bVec)));
}

void SpotLightManager::BuildShadowResources(ID3D11Device* device)
{
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
}
