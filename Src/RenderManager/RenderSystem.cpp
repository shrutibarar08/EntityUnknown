#include "RenderSystem.h"

#include "Utils/Logger/Logger.h"

#include <ranges>

#include "SystemManager/EventQueue/EventQueue.h"
#include "ExceptionManager/RenderException.h"


RenderSystem::RenderSystem(WindowsSystem* winSystem)
	: m_WindowsSystem(winSystem)
{
    //~ Subscribing to events
    EventBus::Subscribe<FullScreenPayload>(EventType::FullScreen,
        [&](const FullScreenPayload& payload)
        {
            ResizeSwapChain(payload.width, payload.height, true);
            LOG_SUCCESS("Managed FullScreen Event");
        });

    EventBus::Subscribe<WindowedScreenPayload>(EventType::WindowedScreen,
        [&](const WindowedScreenPayload& payload)
        {
            ResizeSwapChain(payload.width, payload.height, false);
            LOG_SUCCESS("Managed WindowedScreen Event");
        });

    EventBus::Subscribe<WindowResizePayload>(EventType::WindowResize,
        [&](const WindowResizePayload& payload)
        {
            ResizeSwapChain(payload.width, payload.height, false);
            LOG_SUCCESS("Managed WindowResize Event");
        });
}

bool RenderSystem::OnInit(const SweetLoader& sweetLoader)
{
    if (!QueryAndStoreAdapter()) return false;
    if (!QueryAndStoreMonitorDisplay()) return false;
    if (!BuildRenderer()) return false;

    m_3DCameraId = m_CameraManager.AddCamera("3DCamera");
    m_CameraManager.SetActiveCamera(m_3DCameraId);
    m_CameraManager.GetActiveCamera()->SetAspectRatio(m_WindowsSystem->GetAspectRatio());
    m_CameraManager.GetActiveCamera()->SetTranslationZ(-10);
    m_Render3DQueue = std::make_unique<Render3DQueue>(m_CameraManager.GetCamera(m_3DCameraId), m_Device.Get());
    m_Render2DQueue = std::make_unique<Render2DQueue>(m_CameraManager.GetCamera(m_3DCameraId), m_Device.Get());
	return true;
}

bool RenderSystem::OnTick(float deltaTime)
{
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
	return m_Device.Get();
}

ID3D11DeviceContext* RenderSystem::GetDeviceContext() const
{
	return m_DeviceContext.Get();
}

void RenderSystem::AttachSystemToRender(IRender* sysToRender)
{
	if (m_SystemsToRender.contains(sysToRender->GetAssignedID())) return;
	m_SystemsToRender[sysToRender->GetAssignedID()] = sysToRender;
}

void RenderSystem::RemoveSystemToRender(const IRender* sysToRender)
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
    return m_CurrentAdapterDesc;
}

float RenderSystem::GetRefreshRate() const
{
    return static_cast<float>(m_RefreshRateDenominator) != 0.f
        ? static_cast<float>(m_RefreshRateNumerator) / static_cast<float>(m_RefreshRateDenominator)
        : 60.f;
}

UINT RenderSystem::GetSelectedMSAA() const
{
    return m_CurrentMSAACount;
}

bool RenderSystem::SetMSAA(UINT msaaValue)
{
    if (!m_Device)
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
    HRESULT hr = m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, msaaValue, &quality);
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

    m_RenderTargetView.Reset();
    m_DepthBuffer.Reset();
    m_RenderBuffer.Reset();
    m_DepthStencilView.Reset();
    m_DepthStencilState.Reset();
    m_DeviceContext->OMSetRenderTargets(0u, nullptr, nullptr);

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

    SetOMStates();
    return true;
}

bool RenderSystem::QueryAndStoreAdapter()
{
    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create DXGI Factory.");
        return false;
    }

    UINT adapterIndex = 0;
    SIZE_T maxDedicatedVideoMemory = 0;

    m_Adapters.clear();
    m_SelectedAdapterIndex = -1;

    LOG_INFO("Enumerating GPU adapters...");

    while (true)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        if (dxgiFactory->EnumAdapters(adapterIndex, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wostringstream adapterInfo;
        adapterInfo << L"Adapter " << adapterIndex << L": " << desc.Description;
        adapterInfo << L"\n  VRAM: " << (desc.DedicatedVideoMemory / (1024 * 1024)) << L" MB";
        OutputDebugStringW((adapterInfo.str() + L"\n").c_str()); // Optional for dev view

        m_Adapters.push_back(adapter);

        if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
        {
            maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
            m_SelectedAdapterIndex = static_cast<int>(adapterIndex);
        }

        adapterIndex++;
    }

    if (m_SelectedAdapterIndex == -1)
    {
        THROW_EXCEPTION();
    }

    m_Adapters[m_SelectedAdapterIndex]->GetDesc(&m_CurrentAdapterDesc);

    std::ostringstream selectedInfo;
    selectedInfo << "Selected Adapter [" << m_SelectedAdapterIndex << "]: ";

    std::wstring descWStr = m_CurrentAdapterDesc.Description;
    selectedInfo << std::string(descWStr.begin(), descWStr.end()); // Convert to UTF-8-ish (rough)

    selectedInfo << " | VRAM: " << (m_CurrentAdapterDesc.DedicatedVideoMemory / (1024 * 1024)) << " MB";

    LOG_SUCCESS(selectedInfo.str());

    return true;
}

bool RenderSystem::QueryAndStoreMonitorDisplay()
{
    Microsoft::WRL::ComPtr<IDXGIOutput> output;

    if (m_SelectedAdapterIndex < 0) return false;

    // Try to get output from selected adapter
    HRESULT hr = m_Adapters[m_SelectedAdapterIndex]->EnumOutputs(0, &output);

    if (FAILED(hr) || !output)
    {
        DXGI_ADAPTER_DESC desc{};
        m_Adapters[0]->GetDesc(&desc);
        std::string name = std::string(
            std::begin(desc.Description),
            std::end(desc.Description));

        LOG_WARNING("Selected adapter has no monitor output. Falling back to: " + name);

        if (!m_Adapters.empty())
        {
            hr = m_Adapters[0]->EnumOutputs(0, &output);
        }

        if (FAILED(hr) || !output)
        {
            LOG_FAIL("No monitor/output found on any adapter.");
            return false;
        }
    }
     
    DXGI_OUTPUT_DESC outputDesc;
    output->GetDesc(&outputDesc);

    std::wstring wideName = outputDesc.DeviceName;
    std::string monitorName(wideName.begin(), wideName.end()); // Convert to narrow string
    LOG_INFO("Monitor: " + monitorName);

    DXGI_MODE_DESC desiredMode = {};
    desiredMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_MODE_DESC closestMatch;
    hr = output->FindClosestMatchingMode(&desiredMode, &closestMatch, m_Device.Get());

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to find closest matching display mode.");
        return false;
    }

    UINT refreshRate = closestMatch.RefreshRate.Numerator / closestMatch.RefreshRate.Denominator;
    m_RefreshRateNumerator = closestMatch.RefreshRate.Numerator;
    m_RefreshRateDenominator = closestMatch.RefreshRate.Denominator;

    LOG_SUCCESS("Monitor refresh rate: " + std::to_string(refreshRate) + " Hz");
    return true;
}

bool RenderSystem::QueryAndStoreMSAA()
{
    if (!m_Device)
    {
        LOG_ERROR("Device not initialized. Cannot query MSAA.");
        return false;
    }

    m_SupportedMSAA.clear();
    LOG_INFO("Querying supported MSAA sample counts...");

    for (UINT samples = 1; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples)
    {
        UINT quality = 0;
        if (SUCCEEDED(m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, samples, &quality)) && quality > 0)
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
    if (m_SelectedAdapterIndex < 0 || m_SelectedAdapterIndex >= m_Adapters.size())
    {
        LOG_FAIL("Invalid adapter index for device creation.");
        return false;
    }

    UINT creationFlags = 0;
#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL selectedFeatureLevel = {};

    HRESULT hr = D3D11CreateDevice(
        m_Adapters[m_SelectedAdapterIndex].Get(), // use selected adapter
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_Device,
        &selectedFeatureLevel,
        &m_DeviceContext
    );

    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D11InfoQueue> infoQueue;
    if (SUCCEEDED(m_Device.As(&infoQueue)))
    {
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif

    std::ostringstream oss;
    oss << "D3D11 Device created. Feature Level: 0x" << std::hex << selectedFeatureLevel;
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderSystem::InitSwapChain()
{
    if (!m_Device || m_SelectedAdapterIndex < 0 || !m_WindowsSystem)
    {
        LOG_FAIL("Cannot build swap chain. Missing device, adapter, or window handle.");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    HRESULT hr = m_Adapters[m_SelectedAdapterIndex]->GetParent(__uuidof(IDXGIFactory),
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
        scDesc.BufferDesc.RefreshRate.Numerator = m_RefreshRateNumerator;
        scDesc.BufferDesc.RefreshRate.Denominator = m_RefreshRateDenominator;
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

    hr = dxgiFactory->CreateSwapChain(m_Device.Get(), &scDesc, m_SwapChain.GetAddressOf());
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

    HRESULT hr = m_SwapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(m_RenderBuffer.GetAddressOf())
    );

    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    hr = m_Device->CreateRenderTargetView(
        m_RenderBuffer.Get(),
        nullptr,
        &m_RenderTargetView
    );

    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    LOG_SUCCESS("Render target view created successfully.");
    return true;
}

bool RenderSystem::InitDepthAndStencilView()
{
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = m_PrevWidth;
    depthDesc.Height = m_PrevHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = m_MSAACount;
    depthDesc.SampleDesc.Quality = m_MSAAQuality;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = m_Device->CreateTexture2D(&depthDesc,
        nullptr, &m_DepthBuffer);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Front-facing stencil ops
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Back-facing stencil ops
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc{};
    depthDisabledStencilDesc.DepthEnable = false;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledStencilDesc.StencilEnable = true;
    depthDisabledStencilDesc.StencilReadMask = 0xFF;
    depthDisabledStencilDesc.StencilWriteMask = 0xFF;
    depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = m_Device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_DepthDisabledStencilState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = (m_MSAACount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsvDesc, &m_DepthStencilView);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    LOG_SUCCESS("Depth stencil buffer, state, and view created successfully.");

    SetOMStates();
    return true;
}

bool RenderSystem::InitViewport()
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

    m_DeviceContext->RSSetViewports(1, &viewport);

    LOG_SUCCESS("Build Viewport");

    return true;
}

bool RenderSystem::InitRasterizationState()
{
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;

    HRESULT hr = m_Device->CreateRasterizerState(&rasterDesc, &m_RasterizationState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    m_DeviceContext->RSSetState(m_RasterizationState.Get());

    LOG_SUCCESS("Rasterizer state created with CULL_NONE (both sides visible).");
    return true;
}

void RenderSystem::CleanBuffers()
{
    // Clear render target view with a background color (RGBA)
    const float clearColor[4] = { 0.5f, 0.42f, 0.25f, 1.0f }; // Dark gray background
    if (m_RenderTargetView)
    {
        m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
    }

    // Clear depth-stencil view (depth = 1.0f, stencil = 0)
    if (m_DepthStencilView)
    {
        m_DeviceContext->ClearDepthStencilView(
            m_DepthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f,
            0
        );
    }
}

void RenderSystem::SetOMStates() const
{
    //~ Default State
    if (!m_DeviceContext || !m_RenderTargetView || !m_DepthStencilView)
        return;

    m_DeviceContext->OMSetRenderTargets(1,
        m_RenderTargetView.GetAddressOf(),
        m_DepthStencilView.Get());
}

void RenderSystem::BeginRender()
{
    CleanBuffers();
    for (auto& render : m_SystemsToRender | std::views::values)
    {
        render->RenderBegin();
    }
    Render3DQueue::UpdateVertexConstantBuffer(m_DeviceContext.Get());
    Render2DQueue::UpdateBuffers(m_DeviceContext.Get());
}

void RenderSystem::ExecuteRender()
{
    for (auto& render: m_SystemsToRender | std::views::values)
    {
        render->RenderExecute();
    }
    TurnZBufferOff();
    Render2DQueue::RenderBackgroundAll(m_DeviceContext.Get());
    TurnZBufferOn();
    Render3DQueue::RenderAll(m_DeviceContext.Get());
    TurnZBufferOff();
    Render2DQueue::RenderAll(m_DeviceContext.Get());
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
}

void RenderSystem::TurnZBufferOn() const
{
    if (!m_DepthStencilState) return;
    m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1u);
}

void RenderSystem::TurnZBufferOff() const
{
    if (!m_DepthStencilState) return;
    m_DeviceContext->OMSetDepthStencilState(m_DepthDisabledStencilState.Get(), 1u);
}
