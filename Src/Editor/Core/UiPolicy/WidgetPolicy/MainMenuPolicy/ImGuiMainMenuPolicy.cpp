#include "ImGuiMainMenuPolicy.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/Core/LevelManager/LevelModel.h"


void ImGuiMainMenuPolicy::DrawMainMenu(LevelEditorContext* context)
{
    if (ImGui::BeginMainMenuBar())
    {
        FileMenu(context);
        EditMenu(context);
        ViewMenu(context);
        CreateMenu(context);

        ImGui::EndMainMenuBar();
    }

    //~ Popups
    Popup_NewLevel(context);
    Popup_OpenLevel(context);
    Popup_SaveAs(context);
}

#pragma region MENU_DRAW
void ImGuiMainMenuPolicy::FileMenu(LevelEditorContext* context)
{
    if (!ImGui::BeginMenu("File")) return;

    if (ImGui::MenuItem("New Level", "Ctrl+N"))  m_bShowNewLevelPopup = true;
    if (ImGui::MenuItem("Open Level", "Ctrl+O")) m_bShowOpenPopup = true;
    

    if (ImGui::MenuItem("Save Level", "Ctrl+S")) 
    {
        if (auto* sp = context ? context->GetStoragePolicy() : nullptr)
        {
            // TODO: Save the current level
        }
    }

    if (ImGui::MenuItem("Save Level As", "Ctrl+Shift+S")) m_bShowSaveAsPopup = true;
    
    ImGui::EndMenu();
}

void ImGuiMainMenuPolicy::EditMenu(LevelEditorContext* ctx)
{
    if (!ImGui::BeginMenu("Edit")) return;

    if (ImGui::MenuItem("Undo", "Ctrl+Z"))
    {
        if (auto* cs = ctx ? ctx->GetCommandStack() : nullptr)
            cs->Undo(ctx);
    }
    if (ImGui::MenuItem("Redo", "Ctrl+Y"))
    {
        if (auto* cs = ctx ? ctx->GetCommandStack() : nullptr)
            cs->Redo(ctx);
    }

    ImGui::EndMenu();
}

void ImGuiMainMenuPolicy::ViewMenu(LevelEditorContext* context)
{
    if (!ImGui::BeginMenu("View")) return;

    ImGui::MenuItem("Inspector", nullptr, &m_bShowInspector);
    ImGui::MenuItem("Details", nullptr, &m_bShowDetails);
    ImGui::MenuItem("Assets", nullptr, &m_bShowAssets);

    if (ImGui::MenuItem("Reset Layout"))
    {
        // TODO: Create Dock Reset Default
    }

    ImGui::EndMenu();
}

void ImGuiMainMenuPolicy::CreateMenu(LevelEditorContext* ctx)
{
    if (!ImGui::BeginMenu("Create")) return;
    CreateLightMenu(ctx);
    ImGui::EndMenu();
}

void ImGuiMainMenuPolicy::CreateLightMenu(LevelEditorContext* context)
{
    if (ImGui::BeginMenu("Light"))
    {
        const auto& names = RegistryLight::GetRegisteredNames();
        if (names.empty())
        {
            ImGui::MenuItem("<none>", nullptr, false, false);
        }
        else
        {
            for (const auto& type : names)
            {
                if (ImGui::MenuItem(type.c_str()))
                {
                    auto* level = context->GetLevelManager()->GetActiveLevel();
                    if (level)
                    {
                        auto light = RegistryLight::CreateLight(type);
                        if (auto* cs = context->GetCommandStack())
                        {
                            cs->Execute(
                                std::make_unique<CmdCreateLight>((std::move(light))),
                                context
                            );
                        }                            
                    }
                }
            }
        }
        ImGui::EndMenu();
    }
}
#pragma endregion

#pragma region POPUP_DRAW
void ImGuiMainMenuPolicy::Popup_NewLevel(LevelEditorContext* ctx)
{
    if (m_bShowNewLevelPopup) ImGui::OpenPopup("New Level");

    if (ImGui::BeginPopupModal("New Level", &m_bShowNewLevelPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Name", m_szNewLevelName, IM_ARRAYSIZE(m_szNewLevelName));
        const bool nameEmpty = (m_szNewLevelName[0] == '\0');
        if (nameEmpty) ImGui::TextDisabled("Enter a level name.");

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0))) 
        {
            if (!nameEmpty)
            {
                if (auto* cs = ctx ? ctx->GetCommandStack() : nullptr)
                {
                    cs->Execute(std::make_unique<CmdCreateLevel>(m_szNewLevelName), ctx);
                }
                ImGui::CloseCurrentPopup();
                m_bShowNewLevelPopup = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            m_bShowNewLevelPopup = false;
        }
        ImGui::EndPopup();
    }
}

void ImGuiMainMenuPolicy::Popup_OpenLevel(LevelEditorContext* ctx)
{
    if (m_bShowOpenPopup)
        ImGui::OpenPopup("Open Level");

    if (ImGui::BeginPopupModal("Open Level", &m_bShowOpenPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Path", m_szOpenPath, IM_ARRAYSIZE(m_szOpenPath));

        ImGui::Separator();

        if (ImGui::Button("Open", ImVec2(120, 0))) 
        {
            if (auto* sp = ctx ? ctx->GetStoragePolicy() : nullptr) 
            {
                // TODO: Save Level to local assets if provided from somewhere else
            }
            ImGui::CloseCurrentPopup();
            m_bShowOpenPopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) 
        {
            ImGui::CloseCurrentPopup();
            m_bShowOpenPopup = false;
        }
        ImGui::EndPopup();
    }
}

void ImGuiMainMenuPolicy::Popup_SaveAs(LevelEditorContext* ctx)
{
    if (m_bShowSaveAsPopup)
        ImGui::OpenPopup("Save Level As");

    if (ImGui::BeginPopupModal("Save Level As", &m_bShowSaveAsPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Path", m_szSaveAsPath, IM_ARRAYSIZE(m_szSaveAsPath));

        ImGui::Separator();

        if (ImGui::Button("Save", ImVec2(120, 0))) 
        {
            if (auto* sp = ctx ? ctx->GetStoragePolicy() : nullptr)
            {
                
            }
            ImGui::CloseCurrentPopup();
            m_bShowSaveAsPopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            m_bShowSaveAsPopup = false;
        }

        ImGui::EndPopup();
    }
}

#pragma endregion

void ImGuiMainMenuPolicy::AddRecentPath(const std::string& path)
{
    if (path.empty()) return;
    // remove duplicates
    m_pszRecentPaths.erase(std::remove(m_pszRecentPaths.begin(), m_pszRecentPaths.end(), path), m_pszRecentPaths.end());
    // add to front
    m_pszRecentPaths.insert(m_pszRecentPaths.begin(), path);
    // clamp size
    constexpr size_t kMaxRecent = 8;
    if (m_pszRecentPaths.size() > kMaxRecent)
        m_pszRecentPaths.resize(kMaxRecent);
}
