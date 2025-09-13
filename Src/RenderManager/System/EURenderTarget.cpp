#include "EURenderTarget.h"
#include <cstring>

static DXGI_FORMAT ToTypelessDepthBase(DXGI_FORMAT dsvFmt, bool wantSRV)
{
    if (!wantSRV) return dsvFmt;
    switch (dsvFmt)
    {
    case DXGI_FORMAT_D32_FLOAT:           return DXGI_FORMAT_R32_TYPELESS;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:   return DXGI_FORMAT_R24G8_TYPELESS;
    case DXGI_FORMAT_D16_UNORM:           return DXGI_FORMAT_R16_TYPELESS;
    default:                               return DXGI_FORMAT_UNKNOWN;
    }
}

static DXGI_FORMAT ToDepthSRVFormat(DXGI_FORMAT dsvFmt)
{
    switch (dsvFmt)
    {
    case DXGI_FORMAT_D32_FLOAT:           return DXGI_FORMAT_R32_FLOAT;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:   return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case DXGI_FORMAT_D16_UNORM:           return DXGI_FORMAT_R16_UNORM;
    default:                              return DXGI_FORMAT_UNKNOWN;
    }
}

void EURenderTarget::MakeDebugName_(ID3D11DeviceChild* obj, const wchar_t* name)
{
#if defined(_DEBUG)
    if (!obj || !name) return;
    std::string narrow;
    while (*name) { wchar_t c = *name++; narrow.push_back(static_cast<char>(c & 0xFF)); }
    obj->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)narrow.size(), narrow.data());
#else
    (void)obj; (void)name;
#endif
}

bool EURenderTarget::Create(ID3D11Device* device, const Desc& desc)
{
    Destroy();
    m_IsSwapchain = false;
    m_Desc = desc;

    if (!device || desc.Width == 0 || desc.Height == 0) return false;
    if (desc.ColorFormat == DXGI_FORMAT_UNKNOWN && desc.DepthFormat == DXGI_FORMAT_UNKNOWN) return false;

    if (HasColor() && !CreateColorResources_(device)) return false;
    if (HasDepth() && !CreateDepthResources_(device)) return false;

    return true;
}

bool EURenderTarget::CreateFromSwapChain(ID3D11Device* device,
    IDXGISwapChain* swapChain,
    bool createDepthBuffer,
    DXGI_FORMAT depthFormat,
    const wchar_t* debugName)
{
    Destroy();
    if (!device || !swapChain) return false;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf())))
        return false;

    D3D11_TEXTURE2D_DESC bbDesc{};
    backBuffer->GetDesc(&bbDesc);

    m_IsSwapchain = true;
    m_Desc.Width = bbDesc.Width;
    m_Desc.Height = bbDesc.Height;
    m_Desc.ColorFormat = bbDesc.Format;
    m_Desc.ColorSRV = false;              // swapchain buffers normally lack SRV bind
    m_Desc.SampleCount = bbDesc.SampleDesc.Count;
    m_Desc.SampleQuality = bbDesc.SampleDesc.Quality;
    m_Desc.DebugName = debugName;
    m_Desc.DepthFormat = createDepthBuffer ? depthFormat : DXGI_FORMAT_UNKNOWN;
    m_Desc.DepthSRV = false;

    m_ColorTex = backBuffer;
    if (FAILED(device->CreateRenderTargetView(m_ColorTex.Get(), nullptr, m_RTV.GetAddressOf())))
        return false;
    MakeDebugName_(m_RTV.Get(), L"RTV_Backbuffer");

    if (HasDepth())
    {
        if (!CreateDepthResources_(device))
            return false;
    }
    return true;
}

bool EURenderTarget::CreateShadowMap(ID3D11Device* device, UINT width, UINT height, bool use32f, const wchar_t* debugName)
{
    Desc d{};
    d.Width = width;
    d.Height = height;
    d.ColorFormat = DXGI_FORMAT_UNKNOWN;
    d.DepthFormat = use32f ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
    d.DepthSRV = true;     // we will sample in lighting pass
    d.SampleCount = 1;     // standard shadow maps are single-sample (PCF/PCSS in shader)
    d.DebugName = debugName;
    return Create(device, d);
}

bool EURenderTarget::Resize(ID3D11Device* device, UINT width, UINT height)
{
    if (!device) return false;
    if (width == 0 || height == 0) return false;

    // If swapchain, caller should resize the swapchain and then call CreateFromSwapChain again instead.
    if (m_IsSwapchain) return false;

    if (width == m_Desc.Width && height == m_Desc.Height) return true;

    m_Desc.Width = width;
    m_Desc.Height = height;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> keepSwap; // nothing; just recreate
    m_ColorTex.Reset();
    m_RTV.Reset();
    m_ColorSRV.Reset();
    m_ResolveTex.Reset();
    m_ResolveSRV.Reset();
    m_DepthTex.Reset();
    m_DSV.Reset();
    m_DepthSRV.Reset();

    if (HasColor() && !CreateColorResources_(device)) return false;
    if (HasDepth() && !CreateDepthResources_(device)) return false;
    return true;
}

void EURenderTarget::Destroy()
{
    m_ColorSRV.Reset();
    m_ResolveSRV.Reset();
    m_RTV.Reset();
    m_ColorTex.Reset();

    m_DepthSRV.Reset();
    m_DSV.Reset();
    m_DepthTex.Reset();

    m_ResolveTex.Reset();

    m_IsSwapchain = false;
    m_Desc = Desc{};
}

void EURenderTarget::Bind(ID3D11DeviceContext* ctx)
{
    ID3D11RenderTargetView* rt = m_RTV.Get();
    ctx->OMSetRenderTargets(rt ? 1u : 0u, (rt ? &rt : nullptr), m_DSV.Get());
}

void EURenderTarget::Unbind(ID3D11DeviceContext* ctx)
{
    ID3D11RenderTargetView* nullRTV = nullptr;
    ctx->OMSetRenderTargets(0, &nullRTV, nullptr);
}

void EURenderTarget::ClearColor(ID3D11DeviceContext* ctx, const float rgba[4])
{
    if (m_RTV) ctx->ClearRenderTargetView(m_RTV.Get(), rgba);
}

void EURenderTarget::ClearDepth(ID3D11DeviceContext* ctx, float depth, UINT8 stencil, UINT clearFlags)
{
    if (m_DSV) ctx->ClearDepthStencilView(m_DSV.Get(), clearFlags, depth, stencil);
}

ID3D11ShaderResourceView* EURenderTarget::GetColorSRV(ID3D11Device* device, ID3D11DeviceContext* ctx)
{
    if (!HasColor()) return nullptr;

    // Non-MSAA: return the SRV if we created one
    if (!IsMSAA())
        return m_ColorSRV.Get();

    // MSAA: need to resolve into single-sample texture first, then SRV that
    if (!EnsureResolveTarget_(device)) return nullptr;
    // Resolve from m_ColorTex -> m_ResolveTex
    ctx->ResolveSubresource(m_ResolveTex.Get(), 0, m_ColorTex.Get(), 0, m_Desc.ColorFormat);
    return m_ResolveSRV.Get();
}

bool EURenderTarget::EnsureResolveTarget_(ID3D11Device* device)
{
    if (m_ResolveSRV) return true; // already created

    D3D11_TEXTURE2D_DESC r{};
    r.Width = m_Desc.Width;
    r.Height = m_Desc.Height;
    r.MipLevels = 1;
    r.ArraySize = 1;
    r.Format = m_Desc.ColorFormat;
    r.SampleDesc.Count = 1; // <- single-sample
    r.SampleDesc.Quality = 0;
    r.Usage = D3D11_USAGE_DEFAULT;
    r.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; // keep flexible
    r.CPUAccessFlags = 0;
    r.MiscFlags = 0;

    if (FAILED(device->CreateTexture2D(&r, nullptr, m_ResolveTex.GetAddressOf())))
        return false;
    MakeDebugName_(m_ResolveTex.Get(), L"RT_ResolveTex");

    if (FAILED(device->CreateShaderResourceView(m_ResolveTex.Get(), nullptr, m_ResolveSRV.GetAddressOf())))
        return false;
    MakeDebugName_(m_ResolveSRV.Get(), L"RT_ResolveSRV");
    return true;
}

bool EURenderTarget::CreateColorResources_(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC t{};
    t.Width = m_Desc.Width;
    t.Height = m_Desc.Height;
    t.MipLevels = 1;
    t.ArraySize = 1;
    t.Format = m_Desc.ColorFormat;
    t.SampleDesc.Count = m_Desc.SampleCount;
    t.SampleDesc.Quality = m_Desc.SampleQuality;
    t.Usage = D3D11_USAGE_DEFAULT;
    t.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (!IsMSAA() && m_Desc.ColorSRV)
        t.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    if (FAILED(device->CreateTexture2D(&t, nullptr, m_ColorTex.GetAddressOf())))
        return false;
    MakeDebugName_(m_ColorTex.Get(), m_Desc.DebugName ? m_Desc.DebugName : L"RT_ColorTex");

    // RTV
    if (FAILED(device->CreateRenderTargetView(m_ColorTex.Get(), nullptr, m_RTV.GetAddressOf())))
        return false;
    MakeDebugName_(m_RTV.Get(), L"RTV_Color");

    // SRV (only if non-MSAA)
    if (!IsMSAA() && m_Desc.ColorSRV)
    {
        if (FAILED(device->CreateShaderResourceView(m_ColorTex.Get(), nullptr, m_ColorSRV.GetAddressOf())))
            return false;
        MakeDebugName_(m_ColorSRV.Get(), L"SRV_Color");
    }

    return true;
}

bool EURenderTarget::CreateDepthResources_(ID3D11Device* device)
{
    const bool wantSRV = m_Desc.DepthSRV;
    const DXGI_FORMAT resFormat = ToTypelessDepthBase(m_Desc.DepthFormat, wantSRV);
    if (resFormat == DXGI_FORMAT_UNKNOWN) return false;

    D3D11_TEXTURE2D_DESC t{};
    t.Width = m_Desc.Width;
    t.Height = m_Desc.Height;
    t.MipLevels = 1;
    t.ArraySize = 1;
    t.Format = resFormat;
    t.SampleDesc.Count = m_Desc.SampleCount;
    t.SampleDesc.Quality = m_Desc.SampleQuality;
    t.Usage = D3D11_USAGE_DEFAULT;
    t.BindFlags = D3D11_BIND_DEPTH_STENCIL | (wantSRV ? D3D11_BIND_SHADER_RESOURCE : 0);

    if (FAILED(device->CreateTexture2D(&t, nullptr, m_DepthTex.GetAddressOf())))
        return false;
    MakeDebugName_(m_DepthTex.Get(), m_Desc.DebugName ? m_Desc.DebugName : L"RT_DepthTex");

    // DSV
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
    dsvd.Format = m_Desc.DepthFormat;
    dsvd.ViewDimension = (IsMSAA() ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
    dsvd.Flags = 0;
    if (FAILED(device->CreateDepthStencilView(m_DepthTex.Get(), &dsvd, m_DSV.GetAddressOf())))
        return false;
    MakeDebugName_(m_DSV.Get(), L"DSV_Depth");

    // SRV (for shadow sampling)
    if (wantSRV)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = ToDepthSRVFormat(m_Desc.DepthFormat);
        if (sd.Format == DXGI_FORMAT_UNKNOWN) return false;

        if (IsMSAA())
            sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        else
        {
            sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            sd.Texture2D.MostDetailedMip = 0;
            sd.Texture2D.MipLevels = 1;
        }

        if (FAILED(device->CreateShaderResourceView(m_DepthTex.Get(), &sd, m_DepthSRV.GetAddressOf())))
            return false;
        MakeDebugName_(m_DepthSRV.Get(), L"SRV_Depth");
    }

    return true;
}
