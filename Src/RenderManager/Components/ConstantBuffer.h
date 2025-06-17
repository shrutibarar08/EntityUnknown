#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <type_traits>
#include <DirectXMath.h>

#include "Utils/Logger/Logger.h"


class IConstantBuffer
{
public:
	virtual ~IConstantBuffer() = default;
	virtual ID3D11Buffer* GetBuffer() const = 0;
	virtual void Update(ID3D11DeviceContext* context, const void* data) = 0;
	virtual ID3D11Buffer** GetAddressOf() = 0;
};

template<typename T>
concept ConstantBufferCompatible = std::is_standard_layout_v<T>;

template<ConstantBufferCompatible T>
class ConstantBuffer final: public IConstantBuffer
{
public:
	ConstantBuffer(ID3D11Device* device, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC);

	void Update(ID3D11DeviceContext* context, const void* data) override;
	ID3D11Buffer* GetBuffer() const override;
	ID3D11Buffer** GetAddressOf() override;

private:
	void Update(ID3D11DeviceContext* deviceContext, const T& data);
	void CreateBuffer(ID3D11Device* device);

private:
	D3D11_USAGE m_Usage;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
	T m_InitialData{};
};

template <ConstantBufferCompatible T>
ConstantBuffer<T>::ConstantBuffer(ID3D11Device* device, D3D11_USAGE usage)
	: m_Usage(usage)
{
	CreateBuffer(device);
}

template <ConstantBufferCompatible T>
void ConstantBuffer<T>::Update(ID3D11DeviceContext* context, const void* data)
{
	Update(context, *static_cast<const T*>(data));
}

template <ConstantBufferCompatible T>
inline void ConstantBuffer<T>::Update(ID3D11DeviceContext* deviceContext, const T& data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResources = {};
	HRESULT hr = deviceContext->Map(m_Buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
	memcpy(mappedResources.pData, &data, sizeof(T));
	deviceContext->Unmap(m_Buffer.Get(), 0u);
}

template <ConstantBufferCompatible T>
inline ID3D11Buffer* ConstantBuffer<T>::GetBuffer() const
{
	return m_Buffer.Get();
}

template<ConstantBufferCompatible T>
inline ID3D11Buffer** ConstantBuffer<T>::GetAddressOf()
{
	return m_Buffer.GetAddressOf();
}

template <ConstantBufferCompatible T>
inline void ConstantBuffer<T>::CreateBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = (sizeof(T) + 15) & ~15; // 16-byte alignment
	desc.Usage = m_Usage;
	desc.CPUAccessFlags = (m_Usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	memset(&m_InitialData, 0, sizeof(T));
	initData.pSysMem = &m_InitialData;

	HRESULT hr = device->CreateBuffer(&desc, &initData, &m_Buffer);

	int size = (sizeof(T) + 15) & ~15;

	LOG_INFO("Created Constant Buffer With Size: " + std::to_string(size));
}
