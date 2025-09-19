// PostChain.cpp
#include "PostChain.h"

#include <algorithm>
#include <utility>
#include <memory>

#include "Utils/HelperFunctions.h"

#include "RenderManager/Components/ShaderResource/VertexShader/VertexShader.h"
#include "RenderManager/PostEffect/PostEffect.h"

namespace
{
    static inline float JGetFloat(const nlohmann::json& j, const char* k, float def)
    {
        const auto it = j.find(k); return (it != j.end() && it->is_number()) ? static_cast<float>(it->get<double>()) : def;
    }
    static inline int JGetInt(const nlohmann::json& j, const char* k, int def)
    {
        const auto it = j.find(k); return (it != j.end() && it->is_number_integer()) ? it->get<int>() : def;
    }
    static inline bool JGetBool(const nlohmann::json& j, const char* k, bool def)
    {
        const auto it = j.find(k); return (it != j.end() && it->is_boolean()) ? it->get<bool>() : def;
    }

    static inline DirectX::XMFLOAT3 JGetF3(const nlohmann::json& arr, DirectX::XMFLOAT3 def)
    {
        if (!arr.is_array() || arr.size() < 3) return def;
        return DirectX::XMFLOAT3(
            static_cast<float>(arr[0].get<double>()),
            static_cast<float>(arr[1].get<double>()),
            static_cast<float>(arr[2].get<double>()));
    }
    static inline DirectX::XMFLOAT4 JGetF4(const nlohmann::json& arr, DirectX::XMFLOAT4 def) 
    {
        if (!arr.is_array() || arr.size() < 4) return def;
        return DirectX::XMFLOAT4(
            static_cast<float>(arr[0].get<double>()),
            static_cast<float>(arr[1].get<double>()),
            static_cast<float>(arr[2].get<double>()),
            static_cast<float>(arr[3].get<double>()));
    }
    static inline DirectX::XMMATRIX JGetM4(const nlohmann::json& arr) 
    {
        if (!arr.is_array() || arr.size() != 4) return DirectX::XMMatrixIdentity();
        float m[16]{};
        for (int r = 0; r < 4; ++r) {
            const nlohmann::json& row = arr[r];
            if (!row.is_array() || row.size() != 4) return DirectX::XMMatrixIdentity();
            for (int c = 0; c < 4; ++c) m[r * 4 + c] = static_cast<float>(row[c].get<double>());
        }
        return DirectX::XMMATRIX(
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]);
    }


    static inline POSTFX_COMMON_PS_CB FromJsonCB(const nlohmann::json& j)
    {
        POSTFX_COMMON_PS_CB cb{};
        cb.iTime = JGetFloat(j, "iTime", 0.f);
        cb.iTimerDelta = JGetFloat(j, "iTimerDelta", 0.f);
        cb.iTimeFrameRate = JGetFloat(j, "iTimeFrameRate", 0.f);
        cb.iFrame = JGetInt(j, "iFrame", 0);

        cb.iResolution = JGetF3(j.value("iResolution", nlohmann::json::array()), DirectX::XMFLOAT3(0, 0, 0));
        cb._padRes = JGetFloat(j, "_padRes", 0.f);

        cb.iMouse = JGetF4(j.value("iMouse", nlohmann::json::array()), DirectX::XMFLOAT4(0, 0, 0, 0));

        cb.ViewMatrix = JGetM4(j.value("ViewMatrix", nlohmann::json::array()));
        cb.ProjectionMatrix = JGetM4(j.value("ProjectionMatrix", nlohmann::json::array()));

        cb.CameraPosition = JGetF3(j.value("CameraPosition", nlohmann::json::array()), DirectX::XMFLOAT3(0, 0, 0));
        cb._padCam = JGetFloat(j, "_padCam", 0.f);

        cb.ExtraPram_1 = JGetF4(j.value("ExtraPram_1", nlohmann::json::array()), DirectX::XMFLOAT4(0, 0, 0, 0));
        cb.ExtraPram_2 = JGetF4(j.value("ExtraPram_2", nlohmann::json::array()), DirectX::XMFLOAT4(0, 0, 0, 0));
        cb.ExtraPram_3 = JGetF4(j.value("ExtraPram_3", nlohmann::json::array()), DirectX::XMFLOAT4(0, 0, 0, 0));
        return cb;
    }

}

PostChain::PostChain()
{
    EU_POST_EFFECT_INIT_DESC desc{};
    desc.BlobDesc.EntryPoint = "main";
    desc.BlobDesc.FilePath   = L"Assets/Shader/Post/Default_PS.hlsl";
    desc.BlobDesc.Target     = "ps_5_0";
    desc.EffectName          = "Default";
    m_defaultID = AddPostEffect(desc);
    m_bDirty = true;

    LOG_WARNING("Default Id Set to:" + std::to_string(m_defaultID));
}

bool PostChain::InitSharedFullscreenVS(ID3D11Device* dev, const std::wstring& vsPath)
{
    if (!dev) return false;
    ID3D11VertexShader* vs = VertexShader::Get(dev, vsPath);
    if (!vs) return false;

    m_FullscreenVS = vs;
    for (auto& [name, data] : m_mapPostEffects)
    {
        if (data.View.IsValid())
        {
            data.View.pEffect->SetSharedFullscreenVS(m_FullscreenVS.Get());
        } 
    }
        
    return true;
}

void PostChain::SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept
{
    m_FullscreenVS = vs;
    for (auto& [name, data] : m_mapPostEffects)
    {
        if (data.View.IsValid())
        {
            data.View.pEffect->SetSharedFullscreenVS(m_FullscreenVS.Get());
        }
    }
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
    for (auto& [name, data] : m_mapPostEffects)
    {
        if (data.View.IsValid())
        {
            data.View.pEffect->SetSharedFullscreenVS(nullptr);
        }
    }
}

bool PostChain::EnsureTargets(ID3D11Device* dev, const EURenderTarget* srcRT)
{
    if (!dev) return false;

    EURenderTarget::Desc d{};
    d.Width = srcRT->Width();
    d.Height = srcRT->Height();
    d.ColorFormat = srcRT->GetDesc().ColorFormat;
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

UINT PostChain::Width             () const noexcept { return m_cachedW; }
UINT PostChain::Height            () const noexcept { return m_cachedH; }
DXGI_FORMAT PostChain::ColorFormat() const noexcept { return m_cachedFmt; }
size_t PostChain::Size            () const noexcept { return m_mapPostEffects.size(); }

ID PostChain::AddPostEffect(const EU_POST_EFFECT_INIT_DESC& desc)
{
    if (ID id = PostEffectPool::Get().IsExits(desc))
    {
        std::string message = "Added Cached Post Effect: " + desc.EffectName;
        LOG_INFO(message);
        AddPostEffect(id);
        return id;
    }
    ID createdID = PostEffectPool::Get().Add(desc);

    EU_POST_EFFECT_SHARED_VIEW sharedData = PostEffectPool::Get().GetEffectByID(createdID);
    //if (!sharedData.IsValid()) return 0u;

    EU_POST_CHAIN_SHARE_VIEW data{};
    data.Built   = false;
    data.Enabled = true;
    data.View    = sharedData;
    data.View.BlobDesc = desc.BlobDesc;

    m_mapPostEffects[createdID] = std::move(data);
    m_bDirty = true;

    std::string message = "Added Post Effect: newly made" + desc.EffectName;
    LOG_INFO(message);

    return createdID;
}

ID PostChain::AddPostEffect(ID effectId)
{
    EU_POST_EFFECT_SHARED_VIEW view = PostEffectPool::Get().GetEffectByID(effectId);
    if (!view.IsValid()) return 0u;
    LOG_WARNING("Was Here");
   if (m_mapPostEffects.contains(effectId)) return 0u;
    EU_POST_CHAIN_SHARE_VIEW data{};
    data.Built      = false;
    data.Enabled    = true;
    data.View       = view;

    m_mapPostEffects[effectId] = std::move(data);
    LOG_INFO("Added Effect!");
    m_bDirty = true;
    return effectId;
}

void PostChain::RemovePostEffect(ID effectId) 
{
    if (!IsAttachedPostEffect(effectId)) return;
    m_mapPostEffects.erase(effectId);
    LOG_INFO("Called to Remove Effect!");
}

IPostEffect* PostChain::GetPostEffect(ID effectID) const
{
    if (!IsAttachedPostEffect(effectID)) return nullptr;
    return m_mapPostEffects.at(effectID).View.pEffect;
}

bool PostChain::IsAttachedPostEffect(ID effectID) const
{
    return m_mapPostEffects.contains(effectID);
}

bool PostChain::SetEnabled(ID effectId, bool enabled)
{
    if (!IsAttachedPostEffect(effectId)) return false;
    m_mapPostEffects[effectId].Enabled = enabled;
    return true;
}

bool PostChain::IsEnabled(ID effectId) const
{
    if (!IsAttachedPostEffect(effectId)) return false;
    return m_mapPostEffects.at(effectId).Enabled;
}

void PostChain::Clear()
{
    m_mapPostEffects.clear();
}

bool PostChain::InitAll(ID3D11Device* device)
{
    if (!device) return false;
    bool ok = true;

    for (const auto& [id, data]: m_mapPostEffects)
    {
        if (!data.View.IsValid()) continue;
        data.View.pEffect->SetSharedFullscreenVS(m_FullscreenVS.Get());
        ok = data.View.pEffect->Init(device) && ok;
    }

    m_bDirty = false;
    return ok;
}

void PostChain::OnResizeAll(uint32_t width, uint32_t height)
{
    for (const auto& [id, data] : m_mapPostEffects)
    {
        if (!data.View.IsValid()) continue;
        data.View.pEffect->OnResize(width, height);
    }
}

void PostChain::Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam)
{
    if (m_mapPostEffects.empty())
    {
        AddPostEffect(m_defaultID); // safe;
        m_bDirty = true;
        std::string message = "Found No Default! FIXING WITH ID" + std::to_string(m_defaultID);
        // LOG_ERROR(message);
        return;
    }
    for (const auto& [id, data] : m_mapPostEffects)
    {
        if (!data.View.IsValid()) continue;
        if (!data.Enabled) continue;

        data.View.pEffect->Update(deltaTime, cam);
    }
}

void PostChain::Execute(ID3D11Device* dev,
    ID3D11DeviceContext* ctx,
    EURenderTarget* srcRT,
    ID3D11DepthStencilState* depthDisabledState,
    ID3D11BlendState* optionalBlendState)
{
    if (!dev || !ctx) return;

    if (m_cachedW != srcRT->Width() || m_cachedH != srcRT->Height() || m_cachedFmt != srcRT->GetDesc().ColorFormat)
    {
        if (!EnsureTargets(dev, srcRT)) return;
    }

    if (!m_FullscreenVS) return;

    if (depthDisabledState) ctx->OMSetDepthStencilState(depthDisabledState, 0);
    ctx->OMSetBlendState(optionalBlendState, nullptr, 0xFFFFFFFF);

    size_t active = 0;
    for (const auto& [id, data] : m_mapPostEffects)
    {
        if (IsEnabled(id) && data.View.pEffect) active++;
    }
    if (active == 0) return;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> prevRTV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> prevDSV;
    ctx->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

    // sanitize stages & IA
    ctx->GSSetShader(nullptr, nullptr, 0);
    ctx->HSSetShader(nullptr, nullptr, 0);
    ctx->DSSetShader(nullptr, nullptr, 0);
    ctx->CSSetShader(nullptr, nullptr, 0);

    ctx->IASetInputLayout(nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(m_FullscreenVS.Get(), nullptr, 0);

    // clear SRVs across all stages before chaining
    {
        ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
        ctx->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
        ctx->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
        ctx->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
        ctx->HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
        ctx->DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
        ctx->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
    }

    EURenderTarget* readRT = srcRT;
    EURenderTarget* writeRTs[2] = { &m_ping, &m_pong };
    int w = 0;

    size_t done = 0;
    for (const auto& [id, data]: m_mapPostEffects)
    {
        if (!data.Enabled || !data.View.IsValid()) continue;
        const bool last = (++done == active);

        if (!last)
        {
            // ensure no SRV aliasing before binding next RTV
            {
                ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
                ctx->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
                ctx->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
                ctx->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
                ctx->HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
                ctx->DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
                ctx->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
            }

            // unbind any previous RTV/DSV before binding next ping/pong target
            {
                ID3D11RenderTargetView* nullRTV[1] = { nullptr };
                ctx->OMSetRenderTargets(1, nullRTV, nullptr);
            }

            writeRTs[w]->Bind(ctx);
            ctx->OMSetBlendState(optionalBlendState, nullptr, 0xFFFFFFFF);

            data.View.pEffect->Render(dev, ctx, *readRT);

            writeRTs[w]->Unbind(ctx);
            readRT = writeRTs[w];
            w ^= 1;
        }
        else
        {
            if (prevRTV)
            {
                ID3D11RenderTargetView* rtv = prevRTV.Get();
                ctx->OMSetRenderTargets(1, &rtv, prevDSV.Get());
            }
            else
            {
                ID3D11RenderTargetView* rtv = m_ping.RTV();
                ctx->OMSetRenderTargets(1, &rtv, nullptr);
            }

            ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
            ctx->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);

            ctx->OMSetBlendState(optionalBlendState, nullptr, 0xFFFFFFFF);
            data.View.pEffect->Render(dev, ctx, *readRT);
        }
    }
}

void PostChain::ExecuteTo(ID3D11Device* dev,
    ID3D11DeviceContext* ctx,
    EURenderTarget* srcRT,
    EURenderTarget* destRT,
    ID3D11DepthStencilState* depthDisabledState,
    ID3D11BlendState* optionalBlendState)
{
    if (!dev || !ctx) return;

    destRT->Bind(ctx);

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<FLOAT>(destRT->Width());
    vp.Height = static_cast<FLOAT>(destRT->Height());
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    Execute(dev, ctx, srcRT, depthDisabledState, optionalBlendState);

    destRT->Unbind(ctx);
}

nlohmann::json PostChain::Serialize() const
{
    nlohmann::json jeffects = nlohmann::json::array();

    for (const auto& kv : m_mapPostEffects)
    {
        const ID id = kv.first;
        const EU_POST_CHAIN_SHARE_VIEW& node = kv.second;
        const EU_POST_EFFECT_SHARED_VIEW& v = node.View;

        const std::string pathUtf8 = WideToUtf8(v.BlobDesc.FilePath);

        nlohmann::json je =
        {
            { "id",     static_cast<std::uint64_t>(id) },
            { "name",   v.EffectName },
            { "path",   pathUtf8 },
            { "entry",  v.BlobDesc.EntryPoint },
            { "target", v.BlobDesc.Target },
            { "enabled", node.Enabled },
            { "built",   node.Built }
        };

        if (v.pEffect)
        {
            nlohmann::json jfx = v.pEffect->GetPostEffectSaveData();
            auto itCB = jfx.find("CB");
            if (itCB != jfx.end() && itCB->is_object())
                je["cb"] = *itCB;
        }

        jeffects.push_back(std::move(je));
    }

    nlohmann::json out =
    {
        { "version",   1 },
        { "defaultId", static_cast<std::uint64_t>(m_defaultID) },
        { "effects",   std::move(jeffects) }
    };
    return out;
}

bool PostChain::Deserialize(const nlohmann::json& j)
{
    m_mapPostEffects.clear();
    m_defaultID = 0;

    auto make_default = [&]()
        {
        EU_POST_EFFECT_INIT_DESC desc{};
        desc.BlobDesc.EntryPoint = "main";
        desc.BlobDesc.Target = "ps_5_0";
#if defined(_WIN32)
        std::u8string u8(reinterpret_cast<const char8_t*>("Assets/Shader/Post/Default_PS.hlsl"));
        desc.BlobDesc.FilePath = std::filesystem::path(u8);
#else
        desc.BlobDesc.FilePath = std::filesystem::path("Assets/Shader/Post/Default_PS.hlsl");
#endif
        desc.EffectName = "Default";
        m_defaultID = AddPostEffect(desc);
        m_bDirty = true;
        LOG_WARNING(std::string("Default Id Set to:") + std::to_string(static_cast<std::uint64_t>(m_defaultID)));
        };

    if (!j.is_object())
    {
        make_default();
        return false;
    }

    const nlohmann::json& arr = j.value("effects", nlohmann::json::array());
    if (!arr.is_array() || arr.empty())
    {
        make_default();
        return false;
    }

    const std::uint64_t savedDefault = j.value("defaultId", 0ull);
    ID newDefaultCandidate = 0;

    try
    {
        for (const nlohmann::json& e : arr)
        {
            if (!e.is_object()) continue;

            const std::uint64_t oldId = e.value("id", 0ull);
            const std::string   name = e.value("name", std::string{});

            std::string pathU8 = e.value("path", std::string{});
            std::string entry = e.value("entry", std::string{});
            std::string target = e.value("target", std::string{});

            // Chain flags
            const bool enabled = e.value("enabled", true);
            const bool built = e.value("built", false);

            if (auto itBlob = e.find("blob"); itBlob != e.end() && itBlob->is_object())
            {
                pathU8 = itBlob->value("path", pathU8);
                entry = itBlob->value("entry", entry);
                target = itBlob->value("target", target);
            }

            if (entry.empty())  entry = "main";
            if (target.empty()) target = "ps_5_0";

            EU_POST_EFFECT_INIT_DESC desc{};
            desc.EffectName = name.empty() ? "Unnamed" : name;
            desc.BlobDesc.EntryPoint = entry;
            desc.BlobDesc.Target = target;
#if defined(_WIN32)
            std::u8string u8(reinterpret_cast<const char8_t*>(pathU8.c_str()));
            desc.BlobDesc.FilePath = std::filesystem::path(u8);
#else
            desc.BlobDesc.FilePath = std::filesystem::path(pathU8);
#endif

            ID nid = AddPostEffect(desc);

            auto it = m_mapPostEffects.find(nid);
            if (it != m_mapPostEffects.end())
            {
                it->second.Enabled = enabled;
                it->second.Built = built;

                if (it->second.View.pEffect)
                {
                    it->second.View.pEffect->SetEnabled(enabled);

                    auto jcbIt = e.find("cb");
                    if (jcbIt != e.end() && jcbIt->is_object())
                    {
                        POSTFX_COMMON_PS_CB cb = FromJsonCB(*jcbIt);
                        it->second.View.pEffect->SetParam(cb);
                    }
                }
            }

            if (oldId == savedDefault)
                newDefaultCandidate = nid;
        }

        if (newDefaultCandidate != 0) m_defaultID = newDefaultCandidate;
        else if (!m_mapPostEffects.empty()) m_defaultID = m_mapPostEffects.begin()->first;
        else { make_default(); return false; }

        m_bDirty = true;
        return true;
    }
    catch (...)
    {
        make_default();
        return false;
    }
}