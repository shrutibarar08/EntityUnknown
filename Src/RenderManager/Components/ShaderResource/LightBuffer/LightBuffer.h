#pragma once

#include <d3d11.h>
#include <ranges>
#include <DirectXMath.h>
#include <format>
#include <algorithm>

#include "SystemManager/PrimaryID.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <typeindex>
#include <wrl/client.h>

#include "RenderManager/Light/ILightData.h"


class ILightBuffer
{
public:
	virtual ~ILightBuffer() = default;
	virtual void AddLight(ILightDataBase* light) = 0;
	virtual void RemoveLight(ILightDataBase* light) = 0;
	virtual void RemoveLight(ID id) = 0;
	virtual void Clear() = 0;

	virtual void Build(ID3D11Device* device) = 0;
	virtual void Update(const DirectX::XMFLOAT3& OwnerPosition, ID3D11DeviceContext* context) = 0;
	virtual void Bind(ID3D11DeviceContext* context, UINT slot) const = 0;
};

struct LightDistance
{
	ID id;
	float distance;

	bool operator<(const LightDistance& rhs) const
	{
		return distance < rhs.distance;
	}
};

template<class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
class LightBuffer final: public ILightBuffer
{
public:
	LightBuffer() = default;
	virtual ~LightBuffer() override = default;

	LightBuffer(const LightBuffer&) = delete;
	LightBuffer(LightBuffer&&) = delete;
	LightBuffer& operator=(const LightBuffer&) = delete;
	LightBuffer& operator=(LightBuffer&&) = delete;

	void AddLight(ILightDataBase* light) override
	{
		if (auto typed = dynamic_cast<ILightData<LightType>*>(light))
			AddLight(typed);
	}
	void RemoveLight(ILightDataBase* light) override
	{
		if (auto typed = dynamic_cast<ILightData<LightType>*>(light))
		{
			RemoveLight(typed);
		}
	}

	void AddLight(ILightData<LightType>* light);
	void RemoveLight(ILightData<LightType>* light);
	void RemoveLight(ID id) override;
	void Clear() override;

	void Build(ID3D11Device* device) override;
	void Update(const DirectX::XMFLOAT3& OwnerPosition, ID3D11DeviceContext* context) override;
	void Bind(ID3D11DeviceContext* context, UINT slot) const override;

	static float CalculateDistance(const DirectX::XMFLOAT3& OwnerPosition, const DirectX::XMFLOAT3& lightPosition);

private:
	DirectX::XMFLOAT3 m_OwnerLastPosition{};
	bool m_FirstInitialization{ false };

	struct CacheLight
	{
		ILightData<LightType>* Light;
		bool Selected = false;
	};
	std::unordered_map<ID, CacheLight> m_Lights{};
	std::vector<LightDistance> m_Queue;
	std::vector<LightType> m_GPUData;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
};

template <class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
void LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::AddLight(ILightData<LightType>* light)
{
	if (!light) return;

	ID id = light->GetAssignedID();
	if (m_Lights.contains(id)) return;
	m_FirstInitialization = false;

	const DirectX::XMFLOAT3 lightPos = light->GetLightPosition();
	const float dist = CalculateDistance(m_OwnerLastPosition, lightPos);

	CacheLight cache{ light, false };

	if constexpr (RESPOND_TO_CLOSEST_ONLY)
	{
		m_Queue.emplace_back(LightDistance{ id, dist });

		// Sort and clamp to MAX_POOL
		std::sort(m_Queue.begin(), m_Queue.end());

		if (m_Queue.size() > MAX_POOL)
		{
			ID removedId = m_Queue.back().id;
			m_Queue.pop_back();
			m_Lights.erase(removedId);
		}

		// Mark selection state
		for (auto& [lid, cl] : m_Lights)
			cl.Selected = false;

		for (const auto& entry : m_Queue)
			m_Lights[entry.id].Selected = true;
	}
	else
	{
		// Only allow up to MAX_POOL
		if (m_Queue.size() >= MAX_POOL)
			return;

		m_Queue.emplace_back(LightDistance{ id, dist });
		cache.Selected = true; // Always selected if added
	}

	m_Lights[id] = cache;
}

template <class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
void LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::RemoveLight(ILightData<LightType>* light)
{
	if (!light) return;
	ID id = light->GetAssignedID();
	if (m_Lights.contains(id))
	{
		m_FirstInitialization = false;
		RemoveLight(id);
		LOG_INFO("Removed Check Ended!");
	}
}

template <class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
void LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::RemoveLight(ID id)
{
	if (!m_Lights.contains(id)) return;
	LOG_INFO("Light Found Going to remove");
	m_Lights.erase(id);

	std::erase_if(m_Queue, [&](const LightDistance& l) 
		{
			LOG_INFO("Removed From Queue");
			return l.id == id;
		});

	for (auto& [_, c] : m_Lights)
		c.Selected = false;

	for (const auto& l : m_Queue)
		m_Lights[l.id].Selected = true;
}

template <class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
void LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::Clear()
{
	m_Lights.clear();
	m_Queue = {};
	LOG_INFO("Cleared Whole Light");
}

template <class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
void LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::Build(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(LightType) * MAX_POOL;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.StructureByteStride = sizeof(LightType);
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	device->CreateBuffer(&desc, nullptr, &m_Buffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = MAX_POOL;

	device->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, &m_SRV);
	LOG_INFO("Light Buffer Built!");
}

template <class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
void LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::Update(const DirectX::XMFLOAT3& OwnerPosition, ID3D11DeviceContext* context)
{
	m_OwnerLastPosition = OwnerPosition;
	m_GPUData.clear();
	m_GPUData.resize(MAX_POOL, {});

	bool isDirty = false;

	for (int i = 0; i < m_Queue.size(); i++)
	{
		auto it = m_Lights.find(m_Queue[i].id);
		if (it == m_Lights.end()) continue;

		ILightData<LightType>* light = it->second.Light;
		m_GPUData[i] = light->GetLightData();
		isDirty = isDirty || light->IsDirty();
		light->UnSetDirty(); // still reset if tracked
	}

	if (!isDirty && m_FirstInitialization) return;
	m_FirstInitialization = true;

	context->UpdateSubresource(
		m_Buffer.Get(),
		0,
		nullptr,
		m_GPUData.data(),
		0,
		0
	);
	LOG_INFO("Update Complete!");
}

template <class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
void LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::Bind(ID3D11DeviceContext* context, UINT slot) const
{
	context->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
}

template<class LightType, int MAX_POOL, bool RESPOND_TO_CLOSEST_ONLY>
inline float LightBuffer<LightType, MAX_POOL, RESPOND_TO_CLOSEST_ONLY>::CalculateDistance(
	const DirectX::XMFLOAT3& OwnerPosition,
	const DirectX::XMFLOAT3& lightPosition)
{
	using namespace DirectX;
	XMVECTOR va = XMLoadFloat3(&OwnerPosition);
	XMVECTOR vb = XMLoadFloat3(&lightPosition);
	return XMVectorGetX(XMVector3LengthSq(va - vb)); // squared distance for performance
}

class LightBufferManager
{
public:
	template<typename BufferType>
	void RegisterBuffer(UINT slot)
	{
		std::type_index type = typeid(BufferType);
		if (m_Buffers.contains(type)) return;

		auto buffer = std::make_unique<BufferType>();
		m_Buffers[type] = std::move(buffer);
		m_BindSlots[type] = slot;
	}

	template<typename BufferType>
	BufferType* GetBuffer()
	{
		std::type_index type = typeid(BufferType);
		auto it = m_Buffers.find(type);
		if (it != m_Buffers.end())
			return static_cast<BufferType*>(it->second.get());
		return nullptr;
	}

	void AddLightToAll(ILightDataBase* light)
	{
		for (auto& buffer : m_Buffers | std::views::values)
			buffer->AddLight(light);
	}

	void RemoveLightFromAll(ILightDataBase* light)
	{
		for (auto& buffer : m_Buffers | std::views::values)
			buffer->RemoveLight(light);
	}

	void BuildAll(ID3D11Device* device)
	{
		for (auto& buffer : m_Buffers | std::views::values)
			buffer->Build(device);
	}

	void RenderAll(const DirectX::XMFLOAT3& ownerPosition, ID3D11DeviceContext* context)
	{
		for (auto& [type, buffer] : m_Buffers)
		{
			buffer->Update(ownerPosition, context);
			buffer->Bind(context, m_BindSlots[type]);
		}
	}

	void ClearAll()
	{
		for (auto& buffer: m_Buffers | std::views::values)
			buffer->Clear();
	}

private:
	std::unordered_map<std::type_index, std::unique_ptr<ILightBuffer>> m_Buffers;
	std::unordered_map<std::type_index, UINT> m_BindSlots;
};
