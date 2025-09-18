// PostChain.cpp
#include "PostChain.h"

#include <algorithm>
#include <utility>

#include "RenderManager/Components/ShaderResource/VertexShader/VertexShader.h"

using Microsoft::WRL::ComPtr;


bool PostChain::InitSharedFullscreenVS(ID3D11Device* dev, const std::wstring& vsPath)
{
    if (!dev) return false;
    ID3D11VertexShader* vs = VertexShader::Get(dev, vsPath);
    if (!vs) return false;

    m_FullscreenVS = vs;
    for (auto& [name, node] : m_nodes)
        if (node.fx) node.fx->SetSharedFullscreenVS(m_FullscreenVS.Get());
    return true;
}

void PostChain::SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept
{
    m_FullscreenVS = vs;
    for (auto& [name, node] : m_nodes)
        if (node.fx) node.fx->SetSharedFullscreenVS(m_FullscreenVS.Get());
}

ID3D11VertexShader* PostChain::GetSharedFullscreenVS() const noexcept
{
    return m_FullscreenVS.Get();
}

bool PostChain::HasSharedFullscreenVS() const noexcept
{
    return m_FullscreenVS != nullptr;
}

void PostChain::ClearSharedFullscreenVS() noexcept
{
    m_FullscreenVS.Reset();
    for (auto& [name, node] : m_nodes)
        if (node.fx) node.fx->SetSharedFullscreenVS(nullptr);
}

bool PostChain::EnsureTargets(ID3D11Device* dev, const EURenderTarget& srcRT)
{
    if (!dev) return false;

    EURenderTarget::Desc d{};
    d.Width = srcRT.Width();
    d.Height = srcRT.Height();
    d.ColorFormat = srcRT.GetDesc().ColorFormat;
    d.ColorSRV = true;
    d.DepthFormat = DXGI_FORMAT_UNKNOWN;
    d.DepthSRV = false;
    d.SampleCount = 1;
    d.DebugName = L"PostChainPingPong";

    m_ping.Destroy();
    m_pong.Destroy();

    const bool okA = m_ping.CreateLevel(dev, d);
    const bool okB = m_pong.CreateLevel(dev, d);
    if (!(okA && okB)) return false;

    m_cachedW = d.Width;
    m_cachedH = d.Height;
    m_cachedFmt = d.ColorFormat;

    OnResizeAll(m_cachedW, m_cachedH);
    return true;
}

bool PostChain::Resize(ID3D11Device* dev, UINT width, UINT height, DXGI_FORMAT colorFmt)
{
    if (!dev || width == 0 || height == 0) return false;

    EURenderTarget::Desc d{};
    d.Width = width;
    d.Height = height;
    d.ColorFormat = colorFmt;
    d.ColorSRV = true;
    d.DepthFormat = DXGI_FORMAT_UNKNOWN;
    d.DepthSRV = false;
    d.SampleCount = 1;
    d.DebugName = L"PostChainPingPong";

    m_ping.Destroy();
    m_pong.Destroy();

    const bool okA = m_ping.CreateLevel(dev, d);
    const bool okB = m_pong.CreateLevel(dev, d);
    if (!(okA && okB)) return false;

    m_cachedW = width;
    m_cachedH = height;
    m_cachedFmt = colorFmt;

    OnResizeAll(width, height);
    return true;
}

UINT PostChain::Width() const noexcept { return m_cachedW; }
UINT PostChain::Height() const noexcept { return m_cachedH; }
DXGI_FORMAT PostChain::ColorFormat() const noexcept { return m_cachedFmt; }

std::string PostChain::Add(std::unique_ptr<IPostEffect> fx, std::string name, bool enabled)
{
    if (!fx) return {};
    if (m_FullscreenVS) fx->SetSharedFullscreenVS(m_FullscreenVS.Get());
    std::string key = UniqueName(std::move(name));
    m_order.push_back(key);
    m_nodes.emplace(key, Node{ std::move(fx), enabled });
    return key;
}

std::string PostChain::InsertAt(std::unique_ptr<IPostEffect> fx, std::string name, size_t index, bool enabled)
{
    if (!fx) return {};
    if (index > m_order.size()) index = m_order.size();
    if (m_FullscreenVS) fx->SetSharedFullscreenVS(m_FullscreenVS.Get());
    std::string key = UniqueName(std::move(name));
    m_order.insert(m_order.begin() + index, key);
    m_nodes.emplace(key, Node{ std::move(fx), enabled });
    return key;
}

std::unique_ptr<IPostEffect> PostChain::Replace(const std::string& name, std::unique_ptr<IPostEffect> fx)
{
    if (!fx) return nullptr;
    auto it = m_nodes.find(name);
    if (it == m_nodes.end()) return nullptr;
    if (m_FullscreenVS) fx->SetSharedFullscreenVS(m_FullscreenVS.Get());
    std::swap(it->second.fx, fx);
    return fx;
}

std::unique_ptr<IPostEffect> PostChain::Remove(const std::string& name)
{
    auto it = m_nodes.find(name);
    if (it == m_nodes.end()) return nullptr;
    auto out = std::move(it->second.fx);
    m_nodes.erase(it);
    m_order.erase(std::remove(m_order.begin(), m_order.end(), name), m_order.end());
    return out;
}

void PostChain::Clear()
{
    m_nodes.clear();
    m_order.clear();
}

IPostEffect* PostChain::Get(const std::string& name) const
{
    auto it = m_nodes.find(name);
    return (it != m_nodes.end() && it->second.fx) ? it->second.fx.get() : nullptr;
}

IPostEffect* PostChain::GetAt(size_t index) const
{
    if (index >= m_order.size()) return nullptr;
    return Get(m_order[index]);
}

bool PostChain::Exists(const std::string& name) const
{
    return m_nodes.find(name) != m_nodes.end();
}

size_t PostChain::Size() const noexcept
{
    return m_order.size();
}

const std::vector<std::string>& PostChain::GetOrder() const noexcept
{
    return m_order;
}

bool PostChain::MoveTo(const std::string& name, size_t newIndex)
{
    auto it = std::find(m_order.begin(), m_order.end(), name);
    if (it == m_order.end()) return false;
    if (newIndex >= m_order.size()) newIndex = m_order.size() - 1;
    std::string tmp = *it;
    m_order.erase(it);
    m_order.insert(m_order.begin() + newIndex, std::move(tmp));
    return true;
}

bool PostChain::SetOrder(const std::vector<std::string>& newOrder)
{
    if (newOrder.size() != m_order.size()) return false;
    std::unordered_map<std::string, int> seen;
    for (const auto& n : newOrder) {
        if (!Exists(n) || ++seen[n] > 1) return false;
    }
    m_order = newOrder;
    return true;
}

bool PostChain::SetEnabled(const std::string& name, bool enabled)
{
    auto it = m_nodes.find(name);
    if (it == m_nodes.end()) return false;
    it->second.enabled = enabled;
    return true;
}

bool PostChain::IsEnabled(const std::string& name) const
{
    auto it = m_nodes.find(name);
    return (it != m_nodes.end()) ? it->second.enabled : false;
}

PostChain::Stats PostChain::GetStats() const
{
    Stats s{};
    s.total = m_order.size();
    for (const auto& n : m_order) {
        auto it = m_nodes.find(n);
        if (it != m_nodes.end() && it->second.enabled && it->second.fx)
            ++s.enabled;
    }
    return s;
}

bool PostChain::InitAll(ID3D11Device* dev)
{
    if (!dev) return false;
    bool ok = true;
    for (const auto& n : m_order) {
        auto it = m_nodes.find(n);
        if (it == m_nodes.end() || !it->second.fx) continue;
        if (m_FullscreenVS) it->second.fx->SetSharedFullscreenVS(m_FullscreenVS.Get());
        ok = it->second.fx->Init(dev) && ok;
    }
    return ok;
}

void PostChain::OnResizeAll(uint32_t width, uint32_t height)
{
    for (const auto& n : m_order) {
        auto it = m_nodes.find(n);
        if (it == m_nodes.end() || !it->second.fx) continue;
        it->second.fx->OnResize(width, height);
    }
}

void PostChain::Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam)
{
    for (const auto& n : m_order) {
        auto it = m_nodes.find(n);
        if (it == m_nodes.end() || !it->second.fx || !it->second.enabled) continue;
        it->second.fx->Update(deltaTime, cam);
    }
}

void PostChain::Execute(ID3D11Device* dev,
    ID3D11DeviceContext* ctx,
    EURenderTarget& srcRT,
    ID3D11DepthStencilState* depthDisabledState,
    ID3D11BlendState* optionalBlendState)
{
    if (!dev || !ctx) return;

    if (m_cachedW != srcRT.Width() || m_cachedH != srcRT.Height() || m_cachedFmt != srcRT.GetDesc().ColorFormat) {
        if (!EnsureTargets(dev, srcRT)) return;
    }

    if (!m_FullscreenVS) return;

    if (depthDisabledState) ctx->OMSetDepthStencilState(depthDisabledState, 0);
    ctx->OMSetBlendState(optionalBlendState, nullptr, 0xFFFFFFFF);

    size_t active = 0;
    for (const auto& n : m_order) {
        auto it = m_nodes.find(n);
        if (it != m_nodes.end() && it->second.enabled && it->second.fx) ++active;
    }
    if (active == 0) return;

    ctx->IASetInputLayout(nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(m_FullscreenVS.Get(), nullptr, 0);

    EURenderTarget* readRT = &srcRT;
    EURenderTarget* writeRTs[2] = { &m_ping, &m_pong };
    int w = 0;

    size_t done = 0;
    for (const auto& n : m_order)
    {
        auto it = m_nodes.find(n);
        if (it == m_nodes.end() || !it->second.fx || !it->second.enabled) continue;

        const bool last = (++done == active);

        if (!last)
        {
            writeRTs[w]->Bind(ctx);
            ctx->OMSetBlendState(optionalBlendState, nullptr, 0xFFFFFFFF);
            it->second.fx->Render(dev, ctx, *readRT);
            writeRTs[w]->Unbind(ctx);
            readRT = writeRTs[w];
            w ^= 1;
        }
        else
        {
            it->second.fx->Render(dev, ctx, *readRT);
        }
    }
}

void PostChain::ExecuteTo(ID3D11Device* dev,
    ID3D11DeviceContext* ctx,
    EURenderTarget& srcRT,
    EURenderTarget& destRT,
    ID3D11DepthStencilState* depthDisabledState,
    ID3D11BlendState* optionalBlendState)
{
    if (!dev || !ctx) return;

    destRT.Bind(ctx);

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<FLOAT>(destRT.Width());
    vp.Height = static_cast<FLOAT>(destRT.Height());
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    Execute(dev, ctx, srcRT, depthDisabledState, optionalBlendState);

    destRT.Unbind(ctx);
}

nlohmann::json PostChain::Serialize() const
{
    nlohmann::json j;
    j["order"] = m_order;

    nlohmann::json enabledMap = nlohmann::json::object();
    for (const auto& n : m_order) {
        auto it = m_nodes.find(n);
        if (it != m_nodes.end()) enabledMap[n] = it->second.enabled;
    }
    j["enabled"] = std::move(enabledMap);

    j["rt"]["width"] = m_cachedW;
    j["rt"]["height"] = m_cachedH;
    j["rt"]["format"] = static_cast<int>(m_cachedFmt);

    j["has_shared_vs"] = (m_FullscreenVS != nullptr);
    return j;
}

bool PostChain::Deserialize(const nlohmann::json& j, ID3D11Device* /*dev*/)
{
    if (!j.is_object()) return false;

    if (j.contains("order") && j["order"].is_array())
    {
        std::vector<std::string> newOrder;
        newOrder.reserve(j["order"].size());
        for (const auto& v : j["order"]) {
            if (v.is_string()) {
                const std::string name = v.get<std::string>();
                if (Exists(name)) newOrder.push_back(name);
            }
        }
        for (const auto& existing : m_order) {
            if (std::find(newOrder.begin(), newOrder.end(), existing) == newOrder.end())
                newOrder.push_back(existing);
        }
        m_order = std::move(newOrder);
    }

    if (j.contains("enabled") && j["enabled"].is_object())
    {
        for (auto it = j["enabled"].begin(); it != j["enabled"].end(); ++it) {
            const std::string name = it.key();
            if (!Exists(name)) continue;
            const bool en = it.value().is_boolean() ? it.value().get<bool>() : true;
            SetEnabled(name, en);
        }
    }

    return true;
}

std::string PostChain::UniqueName(std::string base) const
{
    if (base.empty()) base = "Effect";
    if (!Exists(base)) return base;

    int idx = 1;
    std::string candidate;
    do {
        candidate = base + " (" + std::to_string(idx++) + ")";
    } while (Exists(candidate));
    return candidate;
}
