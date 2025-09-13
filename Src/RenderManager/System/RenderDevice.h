#pragma once
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <vector>
#include <utility>


class RenderAdapter;


typedef struct EU_RENDER_DEVICE_PARAM_DESC
{
    RenderAdapter* adapter{ nullptr };
    UINT creationFlags    { 0 };
    const D3D_FEATURE_LEVEL* featureLevels{ nullptr };
    UINT numFeatureLevels{ 0 };
}EU_RENDER_DEVICE_PARAM_DESC;

class RenderDevice
{
public:
     RenderDevice() = default;
    ~RenderDevice() = default;

    RenderDevice(const RenderDevice&) = delete;
    RenderDevice(RenderDevice&&)      = delete;

    RenderDevice& operator=(const RenderDevice&) = delete;
    RenderDevice& operator=(RenderDevice&&)      = delete;

    bool Create  (const EU_RENDER_DEVICE_PARAM_DESC& params);
    bool Recreate(const EU_RENDER_DEVICE_PARAM_DESC& params);
    bool Recreate();
    void Destroy ();

    // State
    bool IsValid() const { return m_Device != nullptr && m_DeviceContext != nullptr; }
    D3D_FEATURE_LEVEL FeatureLevel() const { return m_FeatureLevel; }

    template<typename Fn>
    void WithDevice(Fn&& fn) const
    {
        if (m_Device) fn(m_Device.Get());
    }

    template<typename Fn>
    void WithContext(Fn&& fn) const
    {
        if (m_DeviceContext) fn(m_DeviceContext.Get());
    }

    ID3D11Device*        GetDevice       () const { return m_Device.Get();        }
    ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext.Get(); }
    IDXGIAdapter*        GetCachedAdapter() const { return m_AdapterCached.Get(); }

private:
    bool CreateInternal(IDXGIAdapter* adapter,
        UINT creationFlags,
        const D3D_FEATURE_LEVEL* levels,
        UINT levelCount);

private:
    Microsoft::WRL::ComPtr<ID3D11Device>        m_Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;
    Microsoft::WRL::ComPtr<IDXGIAdapter>        m_AdapterCached;
    D3D_FEATURE_LEVEL                           m_FeatureLevel{ D3D_FEATURE_LEVEL_9_1 };

    UINT                           m_LastCreationFlags{ 0 };
    std::vector<D3D_FEATURE_LEVEL> m_LastLevels;
};
