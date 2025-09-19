// IPostEffect.h
#pragma once
#include <cstdint>
#include <d3d11.h>

#include "SystemManager/PrimaryID.h"
#include "RenderManager/Interface/IRender.h"

class EURenderTarget;
class LevelEditorContext;

struct alignas(16) POSTFX_COMMON_PS_CB
{
    float                  iTime;
    float                  iTimerDelta;
    float                  iTimeFrameRate;
    int                    iFrame;

    DirectX::XMFLOAT3      iResolution;
    float                  _padRes = 0.0f;

    DirectX::XMFLOAT4      iMouse;

    DirectX::XMMATRIX      ViewMatrix;
    DirectX::XMMATRIX      ProjectionMatrix;

    DirectX::XMFLOAT3      CameraPosition;
    float                  _padCam = 0.0f;

    DirectX::XMFLOAT4      ExtraPram_1;
    DirectX::XMFLOAT4      ExtraPram_2;
    DirectX::XMFLOAT4      ExtraPram_3;
};

static_assert(sizeof(POSTFX_COMMON_PS_CB) % 16 == 0, "POSTFX_COMMON_PS_CB must be 16B aligned");


typedef struct EU_POST_EFFECT_INIT_DESC
{
    std::string       EffectName;
    EU_BLOB_INIT_DESC BlobDesc;
} EU_POST_EFFECT_INIT_DESC;

class IPostEffect : public PrimaryID
{
public:
    IPostEffect(const EU_POST_EFFECT_INIT_DESC& desc)
        : m_descPostEffect{desc}
    {}
    virtual ~IPostEffect() = default;

    virtual bool Init(ID3D11Device* dev)                                     = 0;
    virtual void OnResize(uint32_t width, uint32_t height)                   = 0;
    virtual void Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam) = 0;
    virtual void RenderControlUI(LevelEditorContext* context)                = 0;

    virtual void SetEnabled(bool e) noexcept = 0;
    virtual bool IsEnabled() const noexcept = 0;

    virtual void SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept = 0;
    virtual ID3D11VertexShader* GetSharedFullscreenVS() const noexcept = 0;
    virtual POSTFX_COMMON_PS_CB& GetCBMutable() = 0;
    virtual const POSTFX_COMMON_PS_CB& GetCB() const noexcept = 0;
    virtual void SetParam(const POSTFX_COMMON_PS_CB& commonData) = 0;
    virtual void Render(ID3D11Device* device,
        ID3D11DeviceContext* ctx,
        EURenderTarget& srcRT) = 0;

    virtual const char* GetName() const noexcept                    = 0;
    virtual nlohmann::json GetPostEffectSaveData() const            = 0;

    const EU_POST_EFFECT_INIT_DESC& GetPostEffectDesc() const { return m_descPostEffect; }

protected:
    EU_POST_EFFECT_INIT_DESC m_descPostEffect{};
};
