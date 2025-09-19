#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "RenderManager/ResourcePool/PostEffectPool/PostEffectPool.h"

class LevelEditorContext;

class EngineToolPanelPolicy
{
public:
    bool Init(LevelEditorContext* /*ctx*/);
    void DrawTools(LevelEditorContext* ctx);

private:
    std::string m_filter;

    // rename modal state
    ID          m_pendingRenameID = 0;
    std::string m_renameBuffer;

    // create form state
    EU_POST_EFFECT_INIT_DESC m_newDesc = []
        {
        EU_POST_EFFECT_INIT_DESC d{};
        d.EffectName = "NewPostFX";
        d.BlobDesc.EntryPoint = "main";
        d.BlobDesc.Target = "ps_5_0";
        d.BlobDesc.FilePath = L"Assets/Shader/Post/MyEffect.hlsl";
        return d;
        }();
    std::string m_newDescPathUtf8 = "Assets/Shader/Post/MyEffect.hlsl";

    // toast
    enum class Status { None, Info, Ok, Warn, Error };
    Status      m_status = Status::None;
    std::string m_statusText;
    float       m_statusTimer = 0.0f;

private:

    void DrawHeaderRow();
    void DrawViewTab(LevelEditorContext* ctx);
    void DrawCreateTab(LevelEditorContext* ctx);

    void DrawRenamePopup();
    void DrawToast();


    void ApplyByID(LevelEditorContext* ctx, ID effectId);
    void ApplyByDesc(LevelEditorContext* ctx, const EU_POST_EFFECT_INIT_DESC& d);

    struct PayloadHeader
    {
        uint32_t magic = 0xA5517BAD;
        uint16_t version = 1;
        uint16_t kind = 0;
    };

    static std::string  ToLower(std::string s);
    static std::wstring Utf8ToWide(const std::string& sUtf8);
    static std::string  WideToUtf8(const std::wstring& ws);
    static std::string  NameFromPath(const std::string& relUtf8);

    bool PassesFilter(const EU_POST_EFFECT_SHARED_VIEW& v) const;
    void SetStatus(Status s, const std::string& msg);

    std::string TryExtractUtf8PathFromPayload(const ImGuiPayload* p) const;
    void DropZone(const char* id, const ImVec2& size, const char* hint,
        const std::function<void(const std::string&)>& onDrop);
    
    bool m_showRenamePopup = false;
    bool m_focusRenameOnce = false;
};
