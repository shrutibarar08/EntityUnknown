// PostEffect.cpp
#include "PostEffect.h"

#include "imgui/imgui.h"
#include <d3d11.h>
#include <wrl/client.h>

#include "RenderManager/System/EURenderTarget.h"
#include "RenderManager/Components/ConstantBuffer.h"
#include "RenderManager/Components/ShaderResource/PixelShader/PixelShader.h"

#include "Utils/HelperFunctions.h"

using Microsoft::WRL::ComPtr;

static inline void SetupFullscreenIA(ID3D11DeviceContext* ctx)
{
    ctx->IASetInputLayout(nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

PostEffect::PostEffect(const EU_POST_EFFECT_INIT_DESC& desc)
    : IPostEffect(desc)
{
    m_CBData.iTime          = 0.0f;
    m_CBData.iTimerDelta    = 0.0f;
    m_CBData.iTimeFrameRate = 0.0f;
    m_CBData.iFrame         = 0;
    m_CBData.iResolution    = { 0,0,0 };
    m_CBData.iMouse         = { 0,0,0,0 };
    m_CBData.CameraPosition = { 0,0,0 };
    m_CBData.ExtraPram_1    = { 0,0,0,0 };
    m_CBData.ExtraPram_2    = { 0,0,0,0 };
    m_CBData.ExtraPram_3    = { 0,0,0,0 };
}

bool PostEffect::Init(ID3D11Device* device)
{
    if (!device) return false;

    {
        ID3D11PixelShader* ps = PixelShader::Get(device, &m_descPostEffect.BlobDesc);
        if (!ps) return false;
        m_pPostEffectPixelShader = ps;
    }

    if (!m_Sampler)
    {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device->CreateSamplerState(&sd, m_Sampler.ReleaseAndGetAddressOf())))
            return false;
    }

    if (!m_CommonCB)
        m_CommonCB = std::make_unique<ConstantBuffer<POSTFX_COMMON_PS_CB>>(device);

    m_NeedsReload = false;
    return true;
}

void PostEffect::OnResize(uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;
    m_CBData.iResolution = { float(width), float(height), 0.0f };
}

void PostEffect::Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam)
{
    m_CBData.iTimerDelta = deltaTime;
    m_CBData.iTime += deltaTime;
    m_CBData.iFrame += 1;
    m_CBData.iTimeFrameRate = (deltaTime > 0.f) ? (1.0f / deltaTime) : 0.0f;

    m_CBData.ViewMatrix = cam.ViewMatrix;
    m_CBData.ProjectionMatrix = cam.ProjectionMatrix;
    m_CBData.CameraPosition = cam.CameraPosition;
}

void PostEffect::RenderControlUI(LevelEditorContext* /*context*/)
{
    // TODO: Create How I wanna render it
    //if (ImGui::CollapsingHeader(m_descEffect.EffectName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    //{
    //    ImGui::Checkbox("Enabled", &m_Enabled);

    //    char psPath[512] = {};
    //    strncpy_s(psPath, sizeof(psPath), m_descEffect.EffectPath.c_str(), _TRUNCATE);
    //    if (ImGui::InputText("Pixel Shader Path", psPath, sizeof(psPath)))
    //        m_ShaderFile = psPath;

    //    if (ImGui::Button("Reload PS"))
    //        m_NeedsReload = true;

    //    ImGui::Separator();

    //    ImGui::DragFloat4("Extra 1", &m_CBData.ExtraPram_1.x, 0.01f);
    //    ImGui::DragFloat4("Extra 2", &m_CBData.ExtraPram_2.x, 0.01f);
    //    ImGui::DragFloat4("Extra 3", &m_CBData.ExtraPram_3.x, 0.01f);
    //    ImGui::DragFloat4("iMouse", &m_CBData.iMouse.x, 0.5f);

    //    ImGui::DragFloat("iTime", &m_CBData.iTime, 0.01f);
    //    ImGui::Text("dt: %.4f  fps: %.1f  frame: %d", m_CBData.iTimerDelta, m_CBData.iTimeFrameRate, m_CBData.iFrame);
    //    ImGui::Text("Resolution: %u x %u", m_Width, m_Height);
    //}
}

void PostEffect::Render(ID3D11Device* device,
    ID3D11DeviceContext* ctx,
    EURenderTarget& srcRT)
{
    if (!device || !ctx || !m_Enabled) return;

    if (m_NeedsReload)
    {
        m_pPostEffectPixelShader.Reset();
        if (!Init(device)) return;
    }

    if (!m_pPostEffectPixelShader) return;

    ID3D11ShaderResourceView* src = srcRT.GetColorSRV(device, ctx);
    if (!src) return;

    SetupFullscreenIA(ctx);

    ctx->PSSetShader(m_pPostEffectPixelShader.Get(), nullptr, 0);

    if (m_CommonCB)
    {
        m_CommonCB->Update(ctx, &m_CBData);
        ID3D11Buffer* b0 = m_CommonCB->GetBuffer();
        ctx->PSSetConstantBuffers(0, 1, &b0);
    }

    ctx->PSSetShaderResources(0, 1, &src);
    ID3D11SamplerState* smp = m_Sampler.Get();
    ctx->PSSetSamplers(0, 1, &smp);

    ctx->Draw(3, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->PSSetShaderResources(0, 1, &nullSRV);
}

const char* PostEffect::GetName() const noexcept
{
    return m_descPostEffect.EffectName.c_str();
}

nlohmann::json PostEffect::GetPostEffectSaveData() const
{
    nlohmann::json jDesc = {
        { "EffectName", m_descCreateData.EffectName },
        { "BlobDesc", {
            { "FilePath",  WideToUtf8(m_descCreateData.BlobDesc.FilePath) },
            { "EntryPoint", m_descCreateData.BlobDesc.EntryPoint },
            { "Target",     m_descCreateData.BlobDesc.Target }
        }}
    };

    const auto& cb = m_CBData;

    nlohmann::json jCB = 
    {
        { "iTime",          cb.iTime },
        { "iTimerDelta",    cb.iTimerDelta },
        { "iTimeFrameRate", cb.iTimeFrameRate },
        { "iFrame",         cb.iFrame },

        { "iResolution",    ToJson(cb.iResolution) },
        { "_padRes",        cb._padRes },

        { "iMouse",         ToJson(DirectX::XMFLOAT4(cb.iMouse.x, cb.iMouse.y, cb.iMouse.z, cb.iMouse.w)) },

        { "ViewMatrix",       ToJson(cb.ViewMatrix) },
        { "ProjectionMatrix", ToJson(cb.ProjectionMatrix) },

        { "CameraPosition", ToJson(cb.CameraPosition) },
        { "_padCam",        cb._padCam },

        { "ExtraPram_1",    ToJson(cb.ExtraPram_1) },
        { "ExtraPram_2",    ToJson(cb.ExtraPram_2) },
        { "ExtraPram_3",    ToJson(cb.ExtraPram_3) }
    };

    nlohmann::json out =
    {
        { "version", 1 },
        { "Enabled", m_Enabled },
        { "Desc",    jDesc },
        { "CB",      jCB }
    };
    return out;
}

void PostEffect::SetName(const std::string& name)
{
    m_descPostEffect.EffectName = name;
}

void PostEffect::SetEnabled(bool e) noexcept
{
    m_Enabled = e;
}

bool PostEffect::IsEnabled() const noexcept
{
    return m_Enabled;
}

void PostEffect::RequestReload() noexcept
{
    m_NeedsReload = true;
}

bool PostEffect::NeedsReload() const noexcept
{
    return m_NeedsReload;
}

POSTFX_COMMON_PS_CB& PostEffect::GetCBMutable()
{
    return m_CBData;
}

const POSTFX_COMMON_PS_CB& PostEffect::GetCB() const noexcept
{
    return m_CBData;
}

void PostEffect::SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept
{
    m_SharedFullscreenVS = vs;
}

ID3D11VertexShader* PostEffect::GetSharedFullscreenVS() const noexcept
{
    return m_SharedFullscreenVS.Get();
}
