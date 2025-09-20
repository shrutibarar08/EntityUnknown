#pragma once
#include <string>
#include "imgui/imgui.h"

class LevelEditorContext;
class ID3D11ShaderResourceView;

struct EngineHeaderConfig
{
    float  heightPx = 60.0f;
    float  innerPadX = 8.0f;
    float  innerPadY = 6.0f;
    float  itemSpacingX = 10.0f;
    float  itemSpacingY = 6.0f;
    float  iconTextGap = 6.0f;
    ImVec4 bgColor = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);

    // Icon path
    const char* levelIconPath = "Assets/UI/level_icon.png";
    ImVec2      levelIconSize = ImVec2(22, 22);
    const char* levelLabel    = "Level";

    const char* playIconPath = "Assets/UI/icons/play-button.png";
    const char* stopIconPath = "Assets/UI/icons/stop-button.png";

    ImVec2 playIconSize = ImVec2(22, 22);
    ImVec2 stopIconSize = ImVec2(22, 22);

    ImU32  playAccent = IM_COL32(80, 200, 120, 255);
    ImU32  stopAccent = IM_COL32(220, 80, 80, 255);
    float  iconPad = 4.0f;
};

class EngineHeaderPolicy
{
public:
    EngineHeaderPolicy() = default;
    ~EngineHeaderPolicy() = default;

    bool Init(LevelEditorContext* context);
    void DrawHeader(LevelEditorContext* ctx);

    EngineHeaderConfig& Config() { return m_cfg; }
    const EngineHeaderConfig& Config() const { return m_cfg; }

private:
    void BeginHeaderBar();
    void EndHeaderBar();

    void BeginHeaderTable();
    void EndHeaderTable();

    void DrawCol_LevelMenu(LevelEditorContext* ctx);
    void DrawCol_Spacer();
    void DrawCol_RightSide(LevelEditorContext* ctx);

    void DrawLevelMenu(LevelEditorContext* ctx);
    void OpenCreateLevelModal();
    void DrawCreateLevelModal(LevelEditorContext* ctx);

    bool DrawLevelIconButton();

    bool DrawIconButton(const char* id,
        ID3D11ShaderResourceView* srv,
        ImVec2 size,
        const char* fallbackText,
        ImU32 accent = 0);

private:
    EngineHeaderConfig m_cfg{};

    bool  m_openCreateModalNext = false;
    char  m_newLevelName[128] = "NewLevel";

    ID3D11ShaderResourceView* m_levelIconSRV;
    bool m_iconLoaded = false;

    ID3D11ShaderResourceView* m_playIconSRV{ nullptr };
    ID3D11ShaderResourceView* m_stopIconSRV{ nullptr };
    bool m_playIconLoaded{ false };                    
    bool m_stopIconLoaded{ false };                    
};
