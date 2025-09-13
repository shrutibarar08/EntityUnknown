#include "UiPolicy.h"
#include "Imgui/imgui.h"

#include "LevelEditorManager/Core/EditorContext.h"
#include "LevelEditorManager/Core/Commands/Commands.h"

#include <cstdio>


void ImGuiPolicy::Menu(LevelEditorContext* context)
{
    if (ImGui::BeginMainMenuBar())
    {
        LevelMenu(context);
        EditMenu (context);
        LightMenu(context);
        ImGui::EndMainMenuBar();
    }

    //~ Popups
    LevelCreatePopup(context);
    LightCreatePopup(context);
}

void ImGuiPolicy::DrawInspector(LevelEditorContext* context)
{
    auto* lm = context ? context->GetLevelManager() : nullptr;
    auto* cs = context ? context->GetCommandStack() : nullptr;
    auto* lvl = lm ? lm->GetActiveLevel() : nullptr;

    if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        // ===== Header: level summary & undo/redo =====
        if (!lvl)
        {
            ImGui::TextDisabled("No active level.");
            ImGui::End();
            return;
        }

        ImGui::Text("Level: %s", lvl->GetName().c_str());
        ImGui::SameLine();

        {
            std::size_t lightCount = lvl->GetLightMapData().size();
            ImGui::TextDisabled("Lights: %zu", lightCount);
        }

        ImGui::Separator();

        // ===== Tabs =====
        if (ImGui::BeginTabBar("InspectorTabs"))
        {
            if (ImGui::BeginTabItem("Lights"))
            {
                LightInspector(context);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Level"))
            {
                // List levels + switch (no undo needed)
                if (ImGui::BeginChild("##levels", ImVec2(0, 120), true))
                {
                    for (const auto& n : lm->GetLevelNames())
                    {
                        bool active = lm->IsAnActiveLevel(n);
                        if (ImGui::Selectable(n.c_str(), active))
                            lm->SetActiveLevel(n);
                    }
                    ImGui::EndChild();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Debug"))
            {
                ImGui::TextDisabled("Frame time hooks will code later");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void ImGuiPolicy::LevelMenu(LevelEditorContext* context)
{
    auto* levelManager = context->GetLevelManager();
    if (ImGui::BeginMenu("Levels"))
    {
        if (ImGui::MenuItem("Create Level"))
            m_bShowCreateLevelPopup = true;

        for (const auto& levelName : levelManager->GetLevelNames())
        {
            bool bActive = levelManager->IsAnActiveLevel(levelName);

            if (ImGui::MenuItem(levelName.c_str(), nullptr, bActive))
            {
                if (auto* cs = context->GetCommandStack())
                {
                    cs->Execute(std::make_unique<CmdSetActiveLevel>(levelName), context);
                }
            }
        }
        ImGui::EndMenu();
    }
}

void ImGuiPolicy::LevelCreatePopup(LevelEditorContext* context)
{
    auto* levelManager = context->GetLevelManager();

    if (m_bShowCreateLevelPopup)
        ImGui::OpenPopup("Create Level");

    if (ImGui::BeginPopupModal("Create Level", &m_bShowCreateLevelPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Name", m_szDefaultLevelName, IM_ARRAYSIZE(m_szDefaultLevelName));

        const bool nameEmpty = (m_szDefaultLevelName[0] == '\0');
        const bool nameClashes = (!nameEmpty && levelManager->DoesLevelExists(m_szDefaultLevelName));

        if (nameEmpty)   ImGui::TextDisabled("Enter a level name.");
        if (nameClashes) ImGui::TextColored(ImVec4(1, 0.6f, 0.6f, 1), "Level already exists.");

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (!nameEmpty && !nameClashes)
            {
                if (auto* cs = context->GetCommandStack())
                {
                    cs->Execute(std::make_unique<CmdCreateLevel>(m_szDefaultLevelName), context);
                }

                ImGui::CloseCurrentPopup();
                m_bShowCreateLevelPopup = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            m_bShowCreateLevelPopup = false;
        }

        ImGui::EndPopup();
    }
}

void ImGuiPolicy::EditMenu(LevelEditorContext* context)
{
    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Undo", "Ctrl+Z"))
        {
            if (auto* cs = context->GetCommandStack())
            {
                cs->Undo(context);
            }
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Y"))
        {
            if (auto* cs = context->GetCommandStack())
            {
                cs->Redo(context);
            }
        }
        ImGui::EndMenu();
    }
}

void ImGuiPolicy::LightMenu(LevelEditorContext* context)
{
    auto* lm = context->GetLevelManager();

    if (ImGui::BeginMenu("Lights"))
    {
        for (const auto& lightType : RegistryLight::GetRegisteredNames())
        {
            if (ImGui::MenuItem(lightType.c_str()))
            {
                m_szSelectedLightType   = lightType;
                m_bShowCreateLightPopup = true;
                std::snprintf(m_szNewLightName, sizeof(m_szNewLightName), "light");
            }
        }
        ImGui::EndMenu();
    }
}

void ImGuiPolicy::LightCreatePopup(LevelEditorContext* context)
{
    auto* lm = context->GetLevelManager();
    auto* cs = context->GetCommandStack();

    if (m_bShowCreateLightPopup) ImGui::OpenPopup("Create Light");

    if (ImGui::BeginPopupModal("Create Light", &m_bShowCreateLightPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Type: %s", m_szSelectedLightType.empty() ? "<none>" : m_szSelectedLightType.c_str());

        ImGui::InputText("Name", m_szNewLightName, IM_ARRAYSIZE(m_szNewLightName));
        const bool nameEmpty = (m_szNewLightName[0] == '\0');

        if (nameEmpty) ImGui::TextDisabled("Enter a light name.");

        ImGui::Separator();

        const bool canCreate = !nameEmpty && lm->IsAnyLevelActive() && !m_szSelectedLightType.empty();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (canCreate)
            {
                std::unique_ptr<ILightSource> light = RegistryLight::Create(m_szSelectedLightType);
                
                if (light)
                {
                    light->SetLightName(m_szNewLightName); // Todo: Create Command for this

                    cs->Execute(std::make_unique<CmdCreateLight>(std::move(light)), context);

                    ImGui::CloseCurrentPopup();
                    m_bShowCreateLightPopup = false;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            m_bShowCreateLightPopup = false;
        }

        ImGui::EndPopup();
    }
}

void ImGuiPolicy::LightInspector(LevelEditorContext* context)
{
    auto* lm = context->GetLevelManager();
    auto* cs = context->GetCommandStack();
    auto* lvl = lm ? lm->GetActiveLevel() : nullptr;
    if (!lvl) return;

    std::vector<ID> toDelete;

    ImGui::SeparatorText("Lights");

    for (const auto& [id, L] : lvl->GetLightMapData())
    {
        if (!L) continue;
        ImGui::PushID(L);

        std::string header = L->GetLightName();
        header += " [";
        header += L->GetLightTypeToString();
        header += "]";

        const bool open = ImGui::CollapsingHeader(
            header.c_str(),
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth
        );

        if (open)
        {
            ImGui::Indent();
            L->RenderControlUI(context);
            ImGui::Unindent();
        }

        ImGui::Spacing();

        const ImGuiStyle& style = ImGui::GetStyle();
        const float btn_w = ImGui::CalcTextSize("Delete").x + style.FramePadding.x * 2.0f;

        const float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - btn_w;
        ImGui::SetCursorPosX(x > 0.0f ? x : 0.0f);

        if (ImGui::SmallButton("Delete"))
            toDelete.push_back(id);

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PopID();
    }

    for (ID id : toDelete)
        cs->Execute(std::make_unique<CmdDeleteLight>(id), context);
}
