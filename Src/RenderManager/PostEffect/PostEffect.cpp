// PostEffect.cpp
#include "PostEffect.h"

#include "imgui/imgui.h"
#include <d3d11.h>
#include <wrl/client.h>

#include "RenderManager/System/EURenderTarget.h"
#include "RenderManager/Components/ConstantBuffer.h"
#include "RenderManager/Components/ShaderResource/PixelShader/PixelShader.h"

using Microsoft::WRL::ComPtr;

static inline void SetupFullscreenIA(ID3D11DeviceContext* ctx)
{
    ctx->IASetInputLayout(nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

PostEffect::PostEffect(std::string shaderFile,
    std::string psEntry,
    std::string displayName)
    : m_ShaderFile(std::move(shaderFile))
    , m_PSEntry(std::move(psEntry))
    , m_DisplayName(std::move(displayName))
{
    m_CBData.iTime = 0.0f;
    m_CBData.iTimerDelta = 0.0f;
    m_CBData.iTimeFrameRate = 0.0f;
    m_CBData.iFrame = 0;
    m_CBData.iResolution = { 0,0,0 };
    m_CBData.iMouse = { 0,0,0,0 };
    m_CBData.CameraPosition = { 0,0,0 };
    m_CBData.ExtraPram_1 = { 0,0,0,0 };
    m_CBData.ExtraPram_2 = { 0,0,0,0 };
    m_CBData.ExtraPram_3 = { 0,0,0,0 };
}

bool PostEffect::Init(ID3D11Device* dev)
{
    if (!dev) return false;

    {
        const std::wstring pathW(m_ShaderFile.begin(), m_ShaderFile.end());
        ID3D11PixelShader* ps = PixelShader::Get(dev, pathW);
        if (!ps) return false;
        m_PS = ps;
    }

    if (!m_Sampler)
    {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(dev->CreateSamplerState(&sd, m_Sampler.ReleaseAndGetAddressOf())))
            return false;
    }

    if (!m_CommonCB)
        m_CommonCB = std::make_unique<ConstantBuffer<POSTFX_COMMON_PS_CB>>(dev);

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
    if (ImGui::CollapsingHeader(m_DisplayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Enabled", &m_Enabled);

        char psPath[512] = {};
        strncpy_s(psPath, sizeof(psPath), m_ShaderFile.c_str(), _TRUNCATE);
        if (ImGui::InputText("Pixel Shader Path", psPath, sizeof(psPath)))
            m_ShaderFile = psPath;

        if (ImGui::Button("Reload PS"))
            m_NeedsReload = true;

        ImGui::Separator();

        ImGui::DragFloat4("Extra 1", &m_CBData.ExtraPram_1.x, 0.01f);
        ImGui::DragFloat4("Extra 2", &m_CBData.ExtraPram_2.x, 0.01f);
        ImGui::DragFloat4("Extra 3", &m_CBData.ExtraPram_3.x, 0.01f);
        ImGui::DragFloat4("iMouse", &m_CBData.iMouse.x, 0.5f);

        ImGui::DragFloat("iTime", &m_CBData.iTime, 0.01f);
        ImGui::Text("dt: %.4f  fps: %.1f  frame: %d", m_CBData.iTimerDelta, m_CBData.iTimeFrameRate, m_CBData.iFrame);
        ImGui::Text("Resolution: %u x %u", m_Width, m_Height);
    }
}

void PostEffect::Render(ID3D11Device* device,
    ID3D11DeviceContext* ctx,
    EURenderTarget& srcRT)
{
    if (!device || !ctx || !m_Enabled) return;

    if (m_NeedsReload)
    {
        m_PS.Reset();
        if (!Init(device)) return;
    }

    if (!m_PS) return;

    ID3D11ShaderResourceView* src = srcRT.GetColorSRV(device, ctx);
    if (!src) return;

    SetupFullscreenIA(ctx);

    ctx->PSSetShader(m_PS.Get(), nullptr, 0);

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
    return m_DisplayName.c_str();
}

nlohmann::json PostEffect::GetPostEffectSaveData() const
{
    nlohmann::json j;
    j["name"] = m_DisplayName;
    j["enabled"] = m_Enabled;
    j["ps_path"] = m_ShaderFile;
    j["ps_entry"] = m_PSEntry;

    j["cb"]["ExtraPram_1"] = { m_CBData.ExtraPram_1.x, m_CBData.ExtraPram_1.y, m_CBData.ExtraPram_1.z, m_CBData.ExtraPram_1.w };
    j["cb"]["ExtraPram_2"] = { m_CBData.ExtraPram_2.x, m_CBData.ExtraPram_2.y, m_CBData.ExtraPram_2.z, m_CBData.ExtraPram_2.w };
    j["cb"]["ExtraPram_3"] = { m_CBData.ExtraPram_3.x, m_CBData.ExtraPram_3.y, m_CBData.ExtraPram_3.z, m_CBData.ExtraPram_3.w };
    j["cb"]["iMouse"] = { m_CBData.iMouse.x, m_CBData.iMouse.y, m_CBData.iMouse.z, m_CBData.iMouse.w };
    j["cb"]["resolution"] = { m_Width, m_Height };
    return j;
}

void PostEffect::LoadPostEffectSaveData(const nlohmann::json& json)
{
    if (!json.is_object()) return;

    if (auto it = json.find("name"); it != json.end() && it->is_string())
        m_DisplayName = it->get<std::string>();
    if (auto it = json.find("enabled"); it != json.end() && it->is_boolean())
        m_Enabled = it->get<bool>();
    if (auto it = json.find("ps_path"); it != json.end() && it->is_string())
        m_ShaderFile = it->get<std::string>();
    if (auto it = json.find("ps_entry"); it != json.end() && it->is_string())
        m_PSEntry = it->get<std::string>();

    if (auto it = json.find("cb"); it != json.end() && it->is_object())
    {
        const auto& cb = *it;
        auto loadF4 = [&](const char* key, DirectX::XMFLOAT4& dst) 
        {
            if (auto jt = cb.find(key); jt != cb.end() && jt->is_array() && jt->size() == 4)
            {
                dst.x = (*jt)[0].get<float>();
                dst.y = (*jt)[1].get<float>();
                dst.z = (*jt)[2].get<float>();
                dst.w = (*jt)[3].get<float>();
            }
        };
        loadF4("ExtraPram_1", m_CBData.ExtraPram_1);
        loadF4("ExtraPram_2", m_CBData.ExtraPram_2);
        loadF4("ExtraPram_3", m_CBData.ExtraPram_3);
        if (auto jt = cb.find("iMouse"); jt != cb.end() && jt->is_array() && jt->size() == 4) {
            m_CBData.iMouse.x = (*jt)[0].get<float>();
            m_CBData.iMouse.y = (*jt)[1].get<float>();
            m_CBData.iMouse.z = (*jt)[2].get<float>();
            m_CBData.iMouse.w = (*jt)[3].get<float>();
        }
        if (auto jt = cb.find("resolution"); jt != cb.end() && jt->is_array() && jt->size() >= 2) {
            m_Width = (*jt)[0].get<uint32_t>();
            m_Height = (*jt)[1].get<uint32_t>();
            m_CBData.iResolution = { float(m_Width), float(m_Height), 0.0f };
        }
    }

    m_NeedsReload = true;
}

void PostEffect::SetShaderPath(const std::string& path)
{
    m_ShaderFile = path;
    m_NeedsReload = true;
}

const std::string& PostEffect::GetShaderPath() const noexcept
{
    return m_ShaderFile;
}

void PostEffect::SetPSEntry(std::string entry)
{
    m_PSEntry = std::move(entry);
}

std::string PostEffect::GetPSEntry() const noexcept
{
    return m_PSEntry;
}

void PostEffect::SetName(std::string name)
{
    m_DisplayName = std::move(name);
}

std::string PostEffect::GetFriendlyName() const noexcept
{
    return m_DisplayName;
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
