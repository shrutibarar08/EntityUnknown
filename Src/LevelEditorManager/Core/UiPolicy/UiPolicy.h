#pragma once
#include <string>

class  LevelEditorContext;

class ImGuiPolicy
{
public:
    std::string name() const { return "ImGui"; }
    
    void     Menu               (LevelEditorContext* context);
    void     DrawInspector      (LevelEditorContext* context);

private:
    void LevelMenu       (LevelEditorContext* context);
    void LevelCreatePopup(LevelEditorContext* context);

    void EditMenu        (LevelEditorContext* context);

    //~ Create Light from Menu
    void LightMenu       (LevelEditorContext* context);
    void LightCreatePopup(LevelEditorContext* context);

    // draw all the lights on a light inspector,
    // Can Remove, Update Values or Turn on or off lights
    void LightInspector  (LevelEditorContext* context);

private:
    uint64_t m_SelectedEntity  { 0 };
    int      m_NewLevelCounter { 1 };
    int      m_NewEntityCounter{ 1 };

    //~ Menu
    //~~ Level Related Stuff
    bool m_bShowCreateLevelPopup  { false };
    char m_szDefaultLevelName[64] { "level" };

    //~~ Light Related Stuff
    bool        m_bShowCreateLightPopup{ false };
    std::string m_szSelectedLightType;
    char        m_szNewLightName[64]{ "light" };
    uint64_t    m_SelectedLight{ 0 };

};
