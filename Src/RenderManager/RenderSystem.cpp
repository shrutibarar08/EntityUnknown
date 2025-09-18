#include "RenderSystem.h"

#include "Utils/Logger/Logger.h"
#include "Utils/HelperFunctions.h"

#include <ranges>
#include <filesystem>

#include "SystemManager/EventQueue/EventQueue.h"
#include "ExceptionManager/RenderException.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "RenderQueue/RenderQueue.h"
#include "RenderManager/PostEffect/PostEffect.h"

//~ test
#include "Interface/IRender.h"

RenderSystem::RenderSystem(WindowsSystem* winSystem, PhysicsSystem* physics)
	: m_WindowsSystem(winSystem), m_PhysicsSystem(physics)
{
    //~ Subscribing to events
    EventBus::Subscribe<FullScreenPayload>(EventType::FullScreen,
        [&](const FullScreenPayload& payload)
        {
            ResizeSwapChain(payload.width, payload.height, true);
        });

    EventBus::Subscribe<WindowedScreenPayload>(EventType::WindowedScreen,
        [&](const WindowedScreenPayload& payload)
        {
            ResizeSwapChain(payload.width, payload.height, false);
        });

    EventBus::Subscribe<WindowResizePayload>(EventType::WindowResize,
        [&](const WindowResizePayload& payload)
        {
            ResizeSwapChain(payload.width, payload.height, false);
        });
}

bool RenderSystem::OnInit(const SweetLoader& sweetLoader)
{
    if (!QueryAndStoreAdapter()) return false;
    if (!QueryAndStoreMonitorDisplay()) return false;
    if (!BuildRenderer()) return false;
    
    if (!CreateTestEffectRT()) LOG_ERROR("Failed creating Effect RT");


    m_3DCameraId = m_CameraManager.AddCamera("3DCamera");
    m_CameraManager.SetActiveCamera(m_3DCameraId);
    m_CameraManager.GetActiveCamera()->SetAspectRatio(m_WindowsSystem->GetAspectRatio());
    m_CameraManager.GetActiveCamera()->SetTranslationZ(-10);
    m_CameraManager.GetActiveCamera()->SetWindowsScreenSize(m_WindowsSystem->GetWindowsWidth(), m_WindowsSystem->GetWindowsHeight());

    RenderQueue::Init(
        m_CameraManager.GetCamera(m_3DCameraId),
        m_Device.GetDevice(),
        m_Device.GetDeviceContext(),
        m_PhysicsSystem, &m_EffectRT);

    ImGui_ImplDX11_Init(m_Device.GetDevice(), m_Device.GetDeviceContext());

    if (!InitPostFX())
    {
        LOG_ERROR("Failed To Initialize Post Processing Effects");
    }

	return true;
}

bool RenderSystem::OnFrameUpdate(float deltaTime)
{
    RenderQueue::Get()->Tick(deltaTime);
    BeginRender();
    ExecuteRender();
    EndRender();
	return true;
}

bool RenderSystem::OnExit(SweetLoader& sweetLoader)
{
	return true;
}

std::string RenderSystem::GetSystemName()
{
	return "RenderSystem";
}

ID3D11Device* RenderSystem::GetDevice() const
{
	return m_Device.GetDevice();
}

ID3D11DeviceContext* RenderSystem::GetDeviceContext() const
{
	return m_Device.GetDeviceContext();
}

void RenderSystem::AttachSystemToRender(ISystemRender* sysToRender)
{
	if (m_SystemsToRender.contains(sysToRender->GetAssignedID())) return;
	m_SystemsToRender[sysToRender->GetAssignedID()] = sysToRender;
}

void RenderSystem::RemoveSystemToRender(const ISystemRender* sysToRender)
{
	if (!m_SystemsToRender.contains(sysToRender->GetAssignedID())) return;

	m_SystemsToRender.erase(sysToRender->GetAssignedID());
}

void RenderSystem::RemoveSystemToRender(ID id)
{
	if (!m_SystemsToRender.contains(id)) return;
	m_SystemsToRender.erase(id);
}

DXGI_ADAPTER_DESC RenderSystem::GetAdapterInformation() const
{
    return m_Adapter.GetSelectedDesc();
}

float RenderSystem::GetRefreshRate() const
{
    return static_cast<float>(m_Monitor.RefreshRateDenominator()) != 0.f
        ? static_cast<float>(m_Monitor.RefreshRateNumerator()) / static_cast<float>(m_Monitor.RefreshRateDenominator())
        : 60.f;
}

UINT RenderSystem::GetSelectedMSAA() const
{
    return m_CurrentMSAACount;
}

CameraController* RenderSystem::GetCameraController() const
{
    return m_CameraManager.GetActiveCamera();
}

bool RenderSystem::SetMSAA(UINT msaaValue)
{
    if (!m_Device.IsValid())
    {
        LOG_ERROR("Device not initialized. Cannot set MSAA.");
        return false;
    }

    // Check if requested value is supported
    if (std::find(m_SupportedMSAA.begin(), m_SupportedMSAA.end(), msaaValue) == m_SupportedMSAA.end())
    {
        LOG_FAIL("MSAA " + std::to_string(msaaValue) + "x is not supported on this device.");
        return false;
    }

    UINT quality = 0;
    HRESULT hr = m_Device.GetDevice()->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, msaaValue, &quality);
    if (FAILED(hr) || quality == 0)
    {
        LOG_FAIL("Failed to retrieve MSAA quality level for " + std::to_string(msaaValue) + "x.");
        return false;
    }

    m_MSAACount = msaaValue;
    m_MSAAQuality = quality - 1; // DX expects [0..quality-1]

    m_CurrentMSAACount = msaaValue;

    std::ostringstream oss;
    oss << "MSAA set to " << std::to_string(m_MSAACount) << "x (quality level: " << std::to_string(m_MSAAQuality) << ")";
    LOG_SUCCESS(oss.str());

	return true;
}

std::vector<UINT> RenderSystem::GetAvailableMSAAs() const
{
    return m_SupportedMSAA;
}

void RenderSystem::ResizeSwapChain(UINT width, UINT height, bool fullscreen)
{
    //~ Pre-check to avoid creation if same
    if (m_PrevHeight == height && m_PrevWidth == width) return;
    m_PrevHeight = height;
	m_PrevWidth = width;

    m_MainRT.Destroy();
    m_DepthStencilState.Reset();
    m_Device.GetDeviceContext()->OMSetRenderTargets(0u, nullptr, nullptr);

    if (fullscreen)
    {
        m_SwapChain->SetFullscreenState(TRUE, nullptr);
    }
    else m_SwapChain->SetFullscreenState(FALSE, nullptr);

    HRESULT hr = m_SwapChain->ResizeBuffers(
        0, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
    );

    if (!BuildViewsAndStates()) LOG_ERROR("Failed to build view or state after resizing swap chain");
}

bool RenderSystem::BuildRenderer()
{
    if (!InitDeviceAndContext()) return false;
    if (!QueryAndStoreMSAA()) return false;
    if (!BuildViewsAndStates(true)) return false;

    return true;
}

bool RenderSystem::BuildViewsAndStates(bool buildSwapChain)
{
    if (!SetMSAA(m_CurrentMSAACount)) return false;
    if (buildSwapChain) if (!InitSwapChain()) return false;
    if (!InitRenderTargetView()) return false;
    if (!InitDepthAndStencilView()) return false;
    if (!InitViewport()) return false;
    if (!InitRasterizationState()) return false;
    if (!InitDepthRasterizationState()) return false;
    if (!InitAlphaBlendingState()) return false;

    BindMainRTV();
    return true;
}

bool RenderSystem::QueryAndStoreAdapter()
{
    const int count = m_Adapter.Enumerate();
    if (count <= 0)
    {
        LOG_ERROR("No DXGI adapters found.");
        return false;
    }

    // Try preferred policy; if it fails, fall back to highest VRAM.
    bool ok = m_Adapter.Select(Policy_PreferDiscrete{});
    if (!ok) ok = m_Adapter.Select(Policy_HighestVRAM{});
    if (!ok)
    {
        LOG_ERROR("Failed to select an adapter via policy.");
        return false;
    }

    auto desc = m_Adapter.GetSelectedDesc();

    const std::string name = WideToUTF8(desc.Description);
    const double vramGB = double(desc.DedicatedVideoMemory) / (1024.0 * 1024.0 * 1024.0);

    return true;
}

bool RenderSystem::QueryAndStoreMonitorDisplay()
{
    const int outCount = m_Monitor.Enumerate(m_Adapter);
    if (outCount <= 0)
    {
        auto aDesc = m_Adapter.GetSelectedDesc();
        const std::string aName = WideToUTF8(aDesc.Description);
        //LOG_WARNING("Selected adapter '{}' has no outputs.", aName);
        LOG_FAIL("No monitor/output found on any adapter.");
        return false;
    }

    if (!m_Monitor.Select(OutputPolicy_Primary{}))
    {
        LOG_ERROR("Failed to select an output/monitor via policy.");
        return false;
    }

    const DXGI_OUTPUT_DESC odesc = m_Monitor.GetSelectedOutputDesc();
    const std::string monName = WideToUTF8(odesc.DeviceName);
    //LOG_INFO("Monitor: {}", monName);

    DXGI_MODE_DESC requested{};
    requested.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_MODE_DESC matched{};
    if (!m_Monitor.FindClosestMode(m_Device.GetDevice(), requested, matched))
    {
        LOG_ERROR("Failed to find closest matching display mode.");
        return false;
    }

    const UINT hz = m_Monitor.RefreshRateHzRounded();
    //LOG_SUCCESS("Monitor refresh rate: {} Hz", hz);

    return true;
}

bool RenderSystem::QueryAndStoreMSAA()
{
    if (!m_Device.IsValid())
    {
        LOG_ERROR("Device not initialized. Cannot query MSAA.");
        return false;
    }

    m_SupportedMSAA.clear();
    LOG_INFO("Querying supported MSAA sample counts...");

    for (UINT samples = 1; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples)
    {
        UINT quality = 0;
        if (SUCCEEDED(m_Device.GetDevice()->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, samples, &quality)) && quality > 0)
        {
            m_SupportedMSAA.push_back(samples);

            std::ostringstream oss;
            oss << "  " << samples << "x MSAA supported (Quality levels: " << quality << ")";
            LOG_INFO(oss.str());
        }
    }

    if (m_SupportedMSAA.empty())
    {
        LOG_WARNING("No MSAA sample counts supported.");
        return false;
    }

    std::ostringstream oss;
    oss << "MSAA support query complete. " << m_SupportedMSAA.size() << " levels detected.";
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderSystem::InitDeviceAndContext()
{

    UINT creationFlags = 0;
#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    EU_RENDER_DEVICE_PARAM_DESC desc{};
    desc.creationFlags = creationFlags;
    desc.adapter = &m_Adapter;

    if (!m_Device.CreateLevel(desc))
    {
        LOG_ERROR("Failed to create RenderDevice");
        return false;
    }

    const auto fl = m_Device.FeatureLevel();
    //LOG_SUCCESS("D3D11 Device created. Feature Level: 0x{:X}", static_cast<unsigned>(fl));
    return true;
}

bool RenderSystem::InitSwapChain()
{
    if (!m_Device.IsValid() || !m_WindowsSystem)
    {
        LOG_FAIL("Cannot build swap chain. Missing device, adapter, or window handle.");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    HRESULT hr = m_Adapter.GetSelectedAdapter()->GetParent(__uuidof(IDXGIFactory),
        reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    if (FAILED(hr) || !dxgiFactory)
    {
        THROW_RENDER_EXCEPTION_IF_FAILED(hr);
    }

    RECT rt;
    GetClientRect(m_WindowsSystem->GetWindowHandle(), &rt);

    if (!m_WindowsSystem->GetWindowHandle()) THROW_EXCEPTION();

    m_PrevHeight = rt.bottom - rt.top;
    m_PrevWidth = rt.right - rt.left;

    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.BufferDesc.Width = m_PrevWidth;
    scDesc.BufferDesc.Height = m_PrevHeight;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (m_VSyncEnable)
    {
        scDesc.BufferDesc.RefreshRate.Numerator = m_Monitor.RefreshRateNumerator();
        scDesc.BufferDesc.RefreshRate.Denominator = m_Monitor.RefreshRateDenominator();
    }
    else
    {
        scDesc.BufferDesc.RefreshRate.Numerator = 0;
        scDesc.BufferDesc.RefreshRate.Denominator = 1;
    }
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = m_WindowsSystem->GetWindowHandle();
    scDesc.SampleDesc.Count = m_MSAACount;
    scDesc.SampleDesc.Quality = m_MSAAQuality;
    scDesc.Windowed = m_WindowsSystem->IsFullScreen();
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    hr = dxgiFactory->CreateSwapChain(m_Device.GetDevice(), &scDesc, m_SwapChain.GetAddressOf());
    if (FAILED(hr)) THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    std::ostringstream oss;
    oss << "SwapChain created successfully: "
        << scDesc.BufferDesc.Width << "x" << scDesc.BufferDesc.Height
        << " @ " << (scDesc.BufferDesc.RefreshRate.Numerator / scDesc.BufferDesc.RefreshRate.Denominator)
        << "Hz with " << m_MSAACount << "x MSAA (Q" << m_MSAAQuality << ")";

    LOG_SUCCESS(oss.str());

    if (m_WindowsSystem->IsFullScreen())
    {
        m_SwapChain->SetFullscreenState(TRUE, nullptr);
    }
    else
    {
        m_SwapChain->SetFullscreenState(FALSE, nullptr);
    }

    return true;
}

bool RenderSystem::InitRenderTargetView()
{
    if (!m_SwapChain)
    {
        THROW_EXCEPTION();
    }

    m_MainRT.CreateFromSwapChain(
        m_Device.GetDevice(),
        m_SwapChain.Get(),
        true,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        L"MainBackbuffer"
    );

    LOG_SUCCESS("Render target view created successfully.");
    return true;
}

bool RenderSystem::InitDepthAndStencilView()
{
    if (!m_Device.IsValid()) return false;

    HRESULT hr = S_OK;

    // --- Depth-stencil: ENABLED (depth test & writes) ---
    D3D11_DEPTH_STENCIL_DESC dsDesc{};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    dsDesc.StencilEnable = TRUE;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    dsDesc.BackFace = dsDesc.FrontFace;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;

    m_DepthStencilState.Reset();
    hr = m_Device.GetDevice()->CreateDepthStencilState(&dsDesc, m_DepthStencilState.ReleaseAndGetAddressOf());
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    // --- Depth-stencil: DISABLED (for UI/post, etc.) ---
    D3D11_DEPTH_STENCIL_DESC dsDescDisabled = dsDesc;
    dsDescDisabled.DepthEnable = FALSE;

    m_DepthDisabledStencilState.Reset();
    hr = m_Device.GetDevice()->CreateDepthStencilState(&dsDescDisabled, m_DepthDisabledStencilState.ReleaseAndGetAddressOf());
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    // --- Depth-stencil: READ-ONLY (depth test ON, writes OFF) ---
    D3D11_DEPTH_STENCIL_DESC dsReadOnly = dsDesc;
    dsReadOnly.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    m_DepthReadOnlyState.Reset();
    hr = m_Device.GetDevice()->CreateDepthStencilState(&dsReadOnly, m_DepthReadOnlyState.ReleaseAndGetAddressOf());
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    // --- Ensure main RT has a DSV and bind RTV/DSV + default DS state ---
    if (!m_MainRT.HasDepth() || (m_MainRT.DSV() == nullptr))
    {
        LOG_ERROR("Main render target has no depth buffer/DSV. "
            "Did you call m_MainRT.CreateFromSwapChain(..., /*createDepthBuffer=*/true)?");
        return false;
    }

    auto* ctx = m_Device.GetDeviceContext();
    m_MainRT.Bind(ctx);                                           // sets RTV+DSV
    ctx->OMSetDepthStencilState(m_DepthStencilState.Get(), 1u);   // default: depth on

    LOG_SUCCESS("Depth-stencil states created (enabled/disabled/read-only) and main DSV/RTV bound.");
    return true;
}

bool RenderSystem::InitViewport() const
{
    if (!m_WindowsSystem)
    {
        THROW_EXCEPTION();
    }

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_PrevWidth);
    viewport.Height = static_cast<float>(m_PrevHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_Device.GetDeviceContext()->RSSetViewports(1, &viewport);
    return true;
}

bool RenderSystem::InitRasterizationState()
{
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;

    HRESULT hr = m_Device.GetDevice()->CreateRasterizerState(&rasterDesc, &m_RasterizationState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    m_Device.GetDeviceContext()->RSSetState(m_RasterizationState.Get());

    LOG_SUCCESS("Rasterization state created with CULL_NONE (both sides visible).");
    return true;
}

bool RenderSystem::InitDepthRasterizationState()
{
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.DepthBias = 1024;
    rasterDesc.SlopeScaledDepthBias = 2.0f;
    rasterDesc.DepthBiasClamp = 0.0f;

    HRESULT hr = m_Device.GetDevice()->CreateRasterizerState(&rasterDesc, &m_DepthRasterizationState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    LOG_SUCCESS("Depth rasterization state created for shadow mapping.");
    return true;
}

bool RenderSystem::InitAlphaBlendingState()
{
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;       // Source alpha
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;   // 1 - Source alpha
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;          // Source + Dest
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;             // For alpha channel
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = m_Device.GetDevice()->CreateBlendState(&blendDesc, &m_AlphaBlendingState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    UINT sampleMask = 0xffffffff;
    m_Device.GetDeviceContext()->OMSetBlendState(m_AlphaBlendingState.Get(), blendFactor, sampleMask);

    return true;
}

void RenderSystem::CleanMainRTV()
{
    const float clearColor[4] = { 0.5f, 0.42f, 0.25f, 1.0f };
    m_MainRT.ClearColor(m_Device.GetDeviceContext(), clearColor);
    m_MainRT.ClearDepth(m_Device.GetDeviceContext());
}

void RenderSystem::BindMainRTV()
{
    if (!m_Device.IsValid()) return;
    m_MainRT.Bind(m_Device.GetDeviceContext());
}

void RenderSystem::BeginRender()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (!m_Device.IsValid()) return;

    BindMainRTV();
    if (!InitViewport()) THROW("Failed to set viewport in BeginRender");
    CleanMainRTV();

    for (auto& render : m_SystemsToRender | std::views::values) render->RenderBegin();

    RenderQueue::Get()->Update(m_WindowsSystem->GetWindowsWidth(), m_WindowsSystem->GetWindowsHeight());
    RenderQueue::Get()->UpdateLight();
}

void RenderSystem::ExecuteRender()
{
    for (auto& render: m_SystemsToRender | std::views::values)
    {
        render->RenderExecute();
    }

    auto* ctx = m_Device.GetDeviceContext();
    ctx->RSSetState(m_RasterizationState.Get());

    //m_DeviceContext->RSSetState(m_DepthRasterizationState.Get());
    //RenderQueueSingleton::Get()->RenderShadowCast();

    if (!InitViewport()) THROW("Failed to Set viewport in ExecuteRender");
   
    m_EffectRT.Bind(ctx);
    SetAlphaBlendState();

    // Clear offscreen
    {
        const float offClr[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
        m_EffectRT.ClearColor(ctx, offClr);
        m_EffectRT.ClearDepth(ctx);
    }

    TurnZBufferOff();
    RenderQueue::Get()->RenderBackground();
	TurnZBufferOn();
    RenderQueue::Get()->Render();
    TurnZBufferReadOnly();

    m_MainRT.Bind(m_Device.GetDeviceContext());

    if (m_DepthDisabledStencilState)
        m_Device.GetDeviceContext()->OMSetDepthStencilState(m_DepthDisabledStencilState.Get(), 0);

    //~ Test
    ctx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    RenderQueue::Get()->RenderPostEffects(&m_EffectRT, m_DepthDisabledStencilState.Get());

    SetAlphaBlendState();
    RenderQueue::Get()->RenderFront();

    // Rendering
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void RenderSystem::EndRender()
{
    for (auto& render : m_SystemsToRender | std::views::values)
    {
        render->RenderEnd();
    }

    if (m_SwapChain)
    {
        if (m_VSyncEnable) m_SwapChain->Present(1, 0);
        else m_SwapChain->Present(0, 0);
    }

    if (m_Device.IsValid())
        m_MainRT.Unbind(m_Device.GetDeviceContext());

    RenderQueue::Get()->UnBind();
}

void RenderSystem::TurnZBufferOn() const
{
    if (!m_DepthStencilState) return;
    m_Device.GetDeviceContext()->OMSetDepthStencilState(m_DepthStencilState.Get(), 1u);
}

void RenderSystem::TurnZBufferOff() const
{
    if (!m_DepthStencilState) return;
    m_Device.GetDeviceContext()->OMSetDepthStencilState(m_DepthDisabledStencilState.Get(), 0u);
}

void RenderSystem::TurnZBufferReadOnly() const
{
    if (!m_DepthReadOnlyState) return;
    m_Device.GetDeviceContext()->OMSetDepthStencilState(m_DepthReadOnlyState.Get(), 1u);
}

void RenderSystem::SetAlphaBlendState() const
{
    static float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    static UINT sampleMask = 0xffffffff;
    m_Device.GetDeviceContext()->OMSetBlendState(m_AlphaBlendingState.Get(), blendFactor, sampleMask);
}

bool RenderSystem::CreateTestEffectRT()
{
    EURenderTarget::Desc d{};
    d.Width = m_WindowsSystem->GetWindowsWidth();
    d.Height = m_WindowsSystem->GetWindowsHeight();
    d.ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    d.ColorSRV = true;
    d.DepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d.DepthSRV = false;
    d.SampleCount = 1;
    return m_EffectRT.CreateLevel(m_Device.GetDevice(), d);
}

bool RenderSystem::InitPostFX()
{
    auto* dev = m_Device.GetDevice();
    if (!dev) return false;

    m_PostChain = std::make_unique<PostChain>();

    auto fx = std::make_unique<PostEffect>("Assets/Shader/Post/Post_Test.hlsl", "main", "TestFX");
    auto fx_2 = std::make_unique<PostEffect>("Assets/Shader/Post/GlitchBloom_PS.hlsl", "main", "TestFX2");

    RenderQueue::Get()->SetPostChain(m_PostChain.get());

    m_PostChain->Add(std::move(fx), "TestFX", true);
    m_PostChain->Add(std::move(fx_2), "TestFX2", true);

    return true;
}
