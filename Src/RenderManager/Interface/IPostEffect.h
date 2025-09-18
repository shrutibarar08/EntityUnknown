// IPostEffect.h
#pragma once
#include <cstdint>
#include <d3d11.h>

#include "SystemManager/PrimaryID.h"
#include "RenderManager/Interface/IRender.h"

class EURenderTarget;
class LevelEditorContext;

class __declspec(novtable) IPostEffect : public PrimaryID
{
public:
    virtual ~IPostEffect() = default;

    virtual bool Init(ID3D11Device* dev)                                     = 0;
    virtual void OnResize(uint32_t width, uint32_t height)                   = 0;
    virtual void Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam) = 0;
    virtual void RenderControlUI(LevelEditorContext* context)                = 0;

    virtual void SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept = 0;
    virtual ID3D11VertexShader* GetSharedFullscreenVS() const noexcept = 0;

    virtual void Render(ID3D11Device* device,
        ID3D11DeviceContext* ctx,
        EURenderTarget& srcRT) = 0;

    virtual const char* GetName() const noexcept                    = 0;
    virtual nlohmann::json GetPostEffectSaveData() const            = 0;
    virtual void LoadPostEffectSaveData(const nlohmann::json& json) = 0;
};
