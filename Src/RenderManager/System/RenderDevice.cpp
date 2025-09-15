#include "RenderDevice.h"
#include "RenderAdapter.h"
#include <sstream>
#include "Utils/Logger/Logger.h"
#include "ExceptionManager/RenderException.h"

bool RenderDevice::CreateLevel(const EU_RENDER_DEVICE_PARAM_DESC& params)
{
    if (!params.adapter)
    {
        LOG_ERROR("RenderDevice::Create: params.adapter is null");
        return false;
    }

    auto sel = params.adapter->GetSelectedAdapter();
    if (!sel)
    {
        LOG_ERROR("RenderDevice::Create: adapter has no selected IDXGIAdapter");
        return false;
    }

    static const D3D_FEATURE_LEVEL kDefaultLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    const D3D_FEATURE_LEVEL* levels = params.featureLevels ? params.featureLevels : kDefaultLevels;
    const UINT levelCount = params.numFeatureLevels ? params.numFeatureLevels : (UINT)(sizeof(kDefaultLevels) / sizeof(kDefaultLevels[0]));

    if (!CreateInternal(sel.Get(), params.creationFlags, levels, levelCount))
        return false;

    // Cache for Recreate()
    m_AdapterCached = sel;
    m_LastCreationFlags = params.creationFlags;
    m_LastLevels.assign(levels, levels + levelCount);
    return true;
}

bool RenderDevice::Recreate(const EU_RENDER_DEVICE_PARAM_DESC& params)
{
    Destroy();
    return CreateLevel(params);
}

bool RenderDevice::Recreate()
{
    if (!m_AdapterCached)
    {
        LOG_ERROR("RenderDevice::Recreate: no cached adapter");
        return false;
    }
    if (m_LastLevels.empty())
    {
        LOG_ERROR("RenderDevice::Recreate: no cached feature levels");
        return false;
    }
    Destroy();
    return CreateInternal(m_AdapterCached.Get(),
        m_LastCreationFlags,
        m_LastLevels.data(),
        (UINT)m_LastLevels.size());
}

void RenderDevice::Destroy()
{
    if (m_DeviceContext) m_DeviceContext->ClearState();
    m_DeviceContext.Reset();
    m_Device.Reset();
    m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
}

bool RenderDevice::CreateInternal(IDXGIAdapter* adapter,
    UINT creationFlags,
    const D3D_FEATURE_LEVEL* levels,
    UINT levelCount)
{
#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3D11Device> dev;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx;
    D3D_FEATURE_LEVEL outLevel{};

    const HRESULT hr = D3D11CreateDevice(
        adapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        creationFlags,
        levels,
        levelCount,
        D3D11_SDK_VERSION,
        dev.GetAddressOf(),
        &outLevel,
        ctx.GetAddressOf()
    );

    if (FAILED(hr))
    {
        THROW_RENDER_EXCEPTION_IF_FAILED(hr);
        return false;
    }

#if defined(_DEBUG)
    // Set InfoQueue breaks for corruption/error/warning (if available)
    Microsoft::WRL::ComPtr<ID3D11InfoQueue> infoQueue;
    if (SUCCEEDED(dev.As(&infoQueue)))
    {
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif

    m_Device = dev;
    m_DeviceContext = ctx;
    m_FeatureLevel = outLevel;

    std::ostringstream oss;
    oss << "D3D11 Device created. Feature Level: 0x" << std::hex << outLevel;
    LOG_SUCCESS(oss.str());

    return true;
}
