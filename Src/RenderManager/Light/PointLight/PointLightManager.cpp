#include "PointLightManager.h"
#include "ExceptionManager/RenderException.h"

#include <algorithm>

PointLightManager::PointLightManager(int maxSize, UINT slot)
    : m_Slot(slot), m_MaxBufferSize(maxSize)
{
}

void PointLightManager::AddLight(PointLight* light)
{
    if (!light) return;

    ID id = light->GetAssignedID();
    if (m_Lights.contains(id)) return;

    m_Lights[id] = light;
}

void PointLightManager::RemoveLight(ID id)
{
    auto it = m_Lights.find(id);
    if (it != m_Lights.end())
    {
        m_Lights.erase(it);
    }
}

void PointLightManager::Clear()
{
    m_Lights.clear();
    m_LightQueue.clear();
}

void PointLightManager::Build(ID3D11Device* device)
{
	static_assert(sizeof(POINT_LIGHT_GPU_DATA) % 16 == 0, "SPOT_LIGHT_GPU_DATA must be 16-byte aligned.");

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(POINT_LIGHT_GPU_DATA) * m_MaxBufferSize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.StructureByteStride = sizeof(POINT_LIGHT_GPU_DATA);
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
}

void PointLightManager::Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition)
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
	int count = lightDistances.size() > m_MaxBufferSize ? m_MaxBufferSize : lightDistances.size();

	for (int i = 0; i < count; ++i)
	{
		PointLight* light = m_Lights[lightDistances[i].id];
		m_GPUData.push_back(light->GetLightData());
	}

	// Pad the rest with empty slots if we don't have enough lights
	while (m_GPUData.size() < m_MaxBufferSize)
	{
		m_GPUData.emplace_back(POINT_LIGHT_GPU_DATA{});
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

void PointLightManager::Bind(ID3D11DeviceContext* context) const
{
	context->PSSetShaderResources(m_Slot, 1u, m_SRV.GetAddressOf());
}

int PointLightManager::GetLightCount() const
{
	return static_cast<int>(m_Lights.size());
}

float PointLightManager::CalculateDistance(const DirectX::XMVECTOR& a, const DirectX::XMFLOAT3& b)
{
	const DirectX::XMVECTOR bVec = DirectX::XMLoadFloat3(&b);
	return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(a, bVec)));
}
