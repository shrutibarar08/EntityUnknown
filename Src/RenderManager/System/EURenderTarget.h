#pragma once
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <string>


// A dynamic render-output wrapper for:
//  - Color RTT (with optional SRV)
//  - Depth RTT (with optional SRV) -> shadow mapping
//  - Swapchain backbuffer (present target; no SRV)
//  - MSAA (auto resolve to SRV when requested)
//
class EURenderTarget
{
public:
    struct Desc
    {
        // Size
        UINT Width = 0;
        UINT Height = 0;

        // Color
        DXGI_FORMAT ColorFormat = DXGI_FORMAT_UNKNOWN;
        bool        ColorSRV = true;                // create SRV for color (if non-MSAA)

        // Depth
        // Depth texture will be created typeless when DepthSRV=true to allow SRV sampling.
        // Use DXGI_FORMAT_D32_FLOAT or DXGI_FORMAT_D24_UNORM_S8_UINT for the DSV format.
        DXGI_FORMAT DepthFormat = DXGI_FORMAT_UNKNOWN;
        bool        DepthSRV = false;               // create SRV for depth (shadow map)

        // MSAA
        UINT SampleCount = 1;
        UINT SampleQuality = 0;

        // Misc
        const wchar_t* DebugName = nullptr;
    };

public:
     EURenderTarget() = default;
    ~EURenderTarget() = default;

    EURenderTarget(const EURenderTarget&) = delete;
    EURenderTarget(EURenderTarget&&)      = delete;
    
    EURenderTarget& operator=(const EURenderTarget&) = delete;
    EURenderTarget& operator=(EURenderTarget&&)      = delete;

    bool CreateLevel(ID3D11Device* device, const Desc& desc);
    
    bool CreateFromSwapChain(ID3D11Device* device,
        IDXGISwapChain* swapChain,
        bool createDepthBuffer = true,
        DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
        const wchar_t* debugName = L"SwapChainBackbuffer");

    bool CreateShadowMap(ID3D11Device* device, UINT width, UINT height,
        bool use32f = true, const wchar_t* debugName = L"ShadowMap");

    bool Resize(ID3D11Device* device, UINT width, UINT height);
    void Destroy();
    void Bind(ID3D11DeviceContext* ctx);
    void Unbind(ID3D11DeviceContext* ctx);

    // Clears
    void ClearColor(ID3D11DeviceContext* ctx, const float rgba[4]);
    void ClearDepth(ID3D11DeviceContext* ctx, float depth = 1.0f, UINT8 stencil = 0, UINT clearFlags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);

    // If non-MSAA and ColorSRV=true, returns the color SRV directly.
    ID3D11ShaderResourceView* GetColorSRV(ID3D11Device* device, ID3D11DeviceContext* ctx);

    // Depth SRV (shadow map). Returns null if DepthSRV=false or not depth-only/with depth.
    ID3D11ShaderResourceView* GetDepthSRV() const { return m_DepthSRV.Get(); }

    // Accessors
    ID3D11RenderTargetView*   RTV                  () const { return m_RTV.Get(); }
    ID3D11DepthStencilView*   DSV                  () const { return m_DSV.Get(); }
    ID3D11Texture2D*          ColorTex             () const { return m_ColorTex.Get(); }
    ID3D11Texture2D*          DepthTex             () const { return m_DepthTex.Get(); }
    UINT                      Width                () const { return m_Desc.Width; }
    UINT                      Height               () const { return m_Desc.Height; }
    bool                      IsSwapchainBackbuffer() const { return m_IsSwapchain; }
    bool                      HasColor             () const { return m_Desc.ColorFormat != DXGI_FORMAT_UNKNOWN; }
    bool                      HasDepth             () const { return m_Desc.DepthFormat != DXGI_FORMAT_UNKNOWN; }
    bool                      IsMSAA               () const { return m_Desc.SampleCount > 1; }

    const Desc& GetDesc() const { return m_Desc; }

private:
    // Internal helpers
    bool CreateColorResources_(ID3D11Device* device);
    bool CreateDepthResources_(ID3D11Device* device);
    void MakeDebugName_(ID3D11DeviceChild* obj, const wchar_t* name);

    // Resolve MSAA color into single-sample SRV target (created on-demand)
    bool EnsureResolveTarget_(ID3D11Device* device);

private:
    Desc m_Desc{};

    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_ColorTex;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_RTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ColorSRV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_DepthTex;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   m_DSV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DepthSRV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_ResolveTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ResolveSRV;

    bool m_IsSwapchain = false;
};
