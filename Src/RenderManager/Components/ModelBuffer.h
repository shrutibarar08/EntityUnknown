#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <type_traits>


class IModelBufferSource
{
public:
    IModelBufferSource() = default;
    virtual ~IModelBufferSource() = default;

	IModelBufferSource(const IModelBufferSource&) = default;
    IModelBufferSource(IModelBufferSource&&) = default;
    IModelBufferSource& operator=(const IModelBufferSource&) = default;
    IModelBufferSource& operator=(IModelBufferSource&&) = default;

    virtual Microsoft::WRL::ComPtr<ID3D11Buffer> BuildVertexBuffer(ID3D11Device* device) const = 0;
    virtual Microsoft::WRL::ComPtr<ID3D11Buffer> BuildIndexBuffer(ID3D11Device* device) const = 0;
    virtual UINT GetIndexCount() const = 0;
};

template<typename T>
concept VertexLike = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

template<typename T>
concept IndexType = std::is_integral_v<T>;

//--------------------STATIC MODEL BUFFER --------------------------------------//
template<VertexLike TVertex, size_t VCount, IndexType TIndex, size_t ICount>
class StaticModelBufferSource : public IModelBufferSource
{
public:
    using VertexType = TVertex;

    constexpr StaticModelBufferSource(const TVertex(&vertices)[VCount], const TIndex(&indices)[ICount]);

    Microsoft::WRL::ComPtr<ID3D11Buffer> BuildVertexBuffer(ID3D11Device* device) const override;
    Microsoft::WRL::ComPtr<ID3D11Buffer> BuildIndexBuffer(ID3D11Device* device) const override;
    UINT GetIndexCount() const override;

private:
    const TVertex* m_Vertices;
    const TIndex* m_Indices;
};

template <VertexLike TVertex, size_t VCount, IndexType TIndex, size_t ICount>
constexpr StaticModelBufferSource<TVertex, VCount, TIndex, ICount>::StaticModelBufferSource(
    const TVertex(&vertices)[VCount], const TIndex(&indices)[ICount])
    : m_Vertices(vertices), m_Indices(indices)
{
}

template <VertexLike TVertex, size_t VCount, IndexType TIndex, size_t ICount>
Microsoft::WRL::ComPtr<ID3D11Buffer> StaticModelBufferSource<TVertex, VCount, TIndex, ICount>::BuildVertexBuffer(
	ID3D11Device* device) const
{
    D3D11_BUFFER_DESC desc = { sizeof(TVertex) * VCount, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA data = { m_Vertices };
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    device->CreateBuffer(&desc, &data, &buffer);
    return buffer;
}

template<VertexLike TVertex, size_t VCount, IndexType TIndex, size_t ICount>
inline Microsoft::WRL::ComPtr<ID3D11Buffer> StaticModelBufferSource<TVertex, VCount, TIndex, ICount>::BuildIndexBuffer(ID3D11Device* device) const
{
    D3D11_BUFFER_DESC desc = { sizeof(TIndex) * ICount, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER };
    D3D11_SUBRESOURCE_DATA data = { m_Indices };
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    device->CreateBuffer(&desc, &data, &buffer);
    return buffer;
}

template<VertexLike TVertex, size_t VCount, IndexType TIndex, size_t ICount>
inline UINT StaticModelBufferSource<TVertex, VCount, TIndex, ICount>::GetIndexCount() const
{
    return static_cast<UINT>(ICount);
}

class IModelInstance
{
public:
    IModelInstance() = default;
    virtual ~IModelInstance() = default;

    IModelInstance(const IModelInstance&) = default;
    IModelInstance(IModelInstance&&) = default;
    IModelInstance& operator=(const IModelInstance&) = default;
    IModelInstance& operator=(IModelInstance&&) = default;

    virtual void Init(ID3D11Device* device) = 0;
    virtual void Render(ID3D11DeviceContext* context, D3D11_PRIMITIVE_TOPOLOGY topology) = 0;
    virtual UINT GetIndexCount() const = 0;
};

template<typename BufferSource>
class StaticVBInstance: public IModelInstance
{
public:
    StaticVBInstance(std::shared_ptr<BufferSource> shared);

    void Init(ID3D11Device* device) override;
    void Render(ID3D11DeviceContext* context, D3D11_PRIMITIVE_TOPOLOGY topology) override;
    UINT GetIndexCount() const override;
private:
    std::shared_ptr<BufferSource> m_SharedData;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
    UINT m_IndexCount = 0;
};

template<typename BufferSource>
inline StaticVBInstance<BufferSource>::StaticVBInstance(std::shared_ptr<BufferSource> shared)
    : m_SharedData(std::move(shared))
{}

template<typename BufferSource>
inline void StaticVBInstance<BufferSource>::Init(ID3D11Device* device)
{
    m_VertexBuffer = m_SharedData->BuildVertexBuffer(device);
    m_IndexBuffer = m_SharedData->BuildIndexBuffer(device);
    m_IndexCount = m_SharedData->GetIndexCount();
}

template<typename BufferSource>
inline void StaticVBInstance<BufferSource>::Render(ID3D11DeviceContext* context, D3D11_PRIMITIVE_TOPOLOGY topology)
{
    assert(m_VertexBuffer && "Vertex buffer is null!");
    assert(m_IndexBuffer && "Index buffer is null!");
    assert(m_IndexCount > 0 && "Index count is invalid!");

    UINT stride = sizeof(typename BufferSource::VertexType);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(topology);
    context->DrawIndexed(m_IndexCount, 0, 0);
}

template<typename BufferSource>
inline UINT StaticVBInstance<BufferSource>::GetIndexCount() const
{
    return m_IndexCount;
}

//--------------------DYNAMIC MODEL BUFFER -------------------------------------//
template<VertexLike TVertex, IndexType TIndex>
class DynamicModelBufferSource : public IModelBufferSource
{
public:
    using VertexType = TVertex;

    constexpr DynamicModelBufferSource(size_t maxVertexCount, size_t indexCount)
        : m_MaxVertexCount(maxVertexCount), m_IndexCount(indexCount)
    {
        m_Indices.resize(indexCount);

        for (size_t i = 0; i < indexCount; ++i) m_Indices[i] = static_cast<TIndex>(i);
    }

    void Update(ID3D11DeviceContext* context, const std::vector<TVertex>& vertices)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        context->Map(m_VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, vertices.data(), sizeof(TVertex) * vertices.size());
        context->Unmap(m_VertexBuffer.Get(), 0);
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> BuildVertexBuffer(ID3D11Device* device) const override
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = static_cast<UINT>(sizeof(TVertex) * m_MaxVertexCount);
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        device->CreateBuffer(&desc, nullptr, &m_VertexBuffer);
        return m_VertexBuffer;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> BuildIndexBuffer(ID3D11Device* device) const override
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.ByteWidth = static_cast<UINT>(sizeof(TIndex) * m_IndexCount);
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = m_Indices.data();

        device->CreateBuffer(&desc, &data, &m_IndexBuffer);
        return m_IndexBuffer;
    }

    UINT GetIndexCount() const override
    {
        return static_cast<UINT>(m_IndexCount);
    }
    size_t GetVertexCount() const { return m_MaxVertexCount; }

private:
    size_t m_MaxVertexCount;
    size_t m_IndexCount;
    std::vector<TIndex> m_Indices;

    mutable Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
    mutable Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
};

template<typename BufferSource>
class DynamicInstance
{
public:
    DynamicInstance(std::shared_ptr<BufferSource> shared)
	: m_SharedData(shared)
    {}

    void Init(ID3D11Device* device)
    {
        m_VertexBuffer = m_SharedData->BuildVertexBuffer(device);
        m_IndexBuffer = m_SharedData->BuildIndexBuffer(device);
        m_IndexCount = m_SharedData->GetIndexCount();
    }

    void Render(ID3D11DeviceContext* context, D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        assert(m_VertexBuffer && "Vertex buffer is null!");
        assert(m_IndexBuffer && "Index buffer is null!");
        assert(m_IndexCount > 0 && "Index count is invalid!");

        UINT stride = sizeof(typename BufferSource::VertexType);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(topology);
        context->DrawIndexed(m_IndexCount, 0u, 0u);
    }

    UINT GetIndexCount() const
    {
        return m_IndexCount;
    }

    void Update(ID3D11DeviceContext* context, const std::vector<typename BufferSource::VertexType>& vertices)
    {
        m_SharedData->Update(context, vertices);
    }

private:
    std::shared_ptr<BufferSource> m_SharedData;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
    UINT m_IndexCount = 0;
};
