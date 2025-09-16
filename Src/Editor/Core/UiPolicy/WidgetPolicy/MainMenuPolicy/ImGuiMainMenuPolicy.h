#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "Imgui/imgui.h"

#include "Editor/Core/Commands/Commands.h"
#include "SystemManager/Registry/RegistryLight.h"

class LevelEditorContext;
class ILightSource;


class ImGuiMainMenuPolicy
{
public:
    bool Init(LevelEditorContext* context) { return true; }

    void DrawMainMenu(LevelEditorContext* context);

    bool ShowInspector() const noexcept { return m_bShowInspector;   }
    bool ShowDetails  () const noexcept { return m_bShowDetails;     }
    bool ShowAssets   () const noexcept { return m_bShowAssets;      }

    void SetShowInspector(bool v) noexcept { m_bShowInspector = v; }
    void SetShowDetails  (bool v) noexcept { m_bShowDetails = v;   }
    void SetShowAssets   (bool v) noexcept { m_bShowAssets = v;    }

private:
    // Menus
    void FileMenu  (LevelEditorContext* context);
    void EditMenu  (LevelEditorContext* context);
    void ViewMenu  (LevelEditorContext* context);
    void CreateMenu(LevelEditorContext* context);

    //~ SubMenus
    void CreateLightMenu(LevelEditorContext* context);

    // Popups
    void Popup_NewLevel   (LevelEditorContext* context);
    void Popup_OpenLevel  (LevelEditorContext* context);
    void Popup_SaveAs     (LevelEditorContext* context);

    // Helpers
    void AddRecentPath(const std::string& path);

private:
    // ~ View state flags
    bool m_bShowInspector{ true };
    bool m_bShowDetails  { true };
    bool m_bShowAssets   { true };

    // ~ File Related Stuff
    bool  m_bShowNewLevelPopup{ false };
    bool  m_bShowOpenPopup    { false };
    bool  m_bShowSaveAsPopup  { false };
    char  m_szNewLevelName[64]{ "level" };
    char  m_szOpenPath[512]   { "Assets/Level/level.json" };
    char  m_szSaveAsPath[512] { "Assets/Level/level.json" };
    
    std::string              m_szLastSavePath{};
    std::vector<std::string> m_pszRecentPaths;
};
