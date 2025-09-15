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

private:
    EngineHeaderConfig m_cfg{};

    bool  m_openCreateModalNext = false;
    char  m_newLevelName[128] = "NewLevel";

    ID3D11ShaderResourceView* m_levelIconSRV;
    bool m_iconLoaded = false;
};
