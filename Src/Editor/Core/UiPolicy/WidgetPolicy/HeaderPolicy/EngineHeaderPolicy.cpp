#include "EngineHeaderPolicy.h"
#include "imgui/imgui_internal.h"
#include "Editor/Core/EditorContext.h"
#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"


bool EngineHeaderPolicy::Init(LevelEditorContext* context)
{
    if (!m_cfg.levelIconPath) return true;
    auto texture = TextureLoader::GetTexture(m_cfg.levelIconPath);
    m_levelIconSRV = texture.ShaderResourceView;
    m_iconLoaded = texture.IsInitialized();
    return true;
}

void EngineHeaderPolicy::DrawHeader(LevelEditorContext* ctx)
{
    BeginHeaderBar();
    BeginHeaderTable();

    DrawCol_LevelMenu(ctx);
    DrawCol_Spacer();
    DrawCol_RightSide(ctx);

    EndHeaderTable();
    EndHeaderBar();

    DrawCreateLevelModal(ctx);
}

// ---------------- frame orchestration ----------------
void EngineHeaderPolicy::BeginHeaderBar()
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, m_cfg.bgColor);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_cfg.innerPadX, m_cfg.innerPadY));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(m_cfg.itemSpacingX, m_cfg.itemSpacingY));

    ImGui::BeginChild("##engine_header_bar",
        ImVec2(0, m_cfg.heightPx),
        false,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoResize); // no vertical resize
}

void EngineHeaderPolicy::EndHeaderBar()
{
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void EngineHeaderPolicy::BeginHeaderTable()
{
    ImGui::BeginTable("##engine_header_table", 3,
        ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_PadOuterX |
        ImGuiTableFlags_NoSavedSettings);

    ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthStretch, 1.0f);
    ImGui::TableSetupColumn("S", ImGuiTableColumnFlags_WidthStretch, 6.0f);
    ImGui::TableSetupColumn("R", ImGuiTableColumnFlags_WidthStretch, 2.0f);

    ImGui::TableNextRow();
}

void EngineHeaderPolicy::EndHeaderTable()
{
    ImGui::EndTable();
}

void EngineHeaderPolicy::DrawCol_LevelMenu(LevelEditorContext* ctx)
{
    ImGui::TableNextColumn();
    ImGui::PushID("LevelMenu");

    if (DrawLevelIconButton())
        ImGui::OpenPopup("##level_popup");

    DrawLevelMenu(ctx);
    ImGui::PopID();
}

void EngineHeaderPolicy::DrawCol_Spacer()
{
    ImGui::TableNextColumn();
}

void EngineHeaderPolicy::DrawCol_RightSide(LevelEditorContext* /*ctx*/)
{
    ImGui::TableNextColumn();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
    ImGui::TextDisabled("Status: Ready");
}

void EngineHeaderPolicy::DrawLevelMenu(LevelEditorContext* ctx)
{
    if (ImGui::BeginPopup("##level_popup", ImGuiWindowFlags_NoMove))
    {
        // Save / Reload
        if (ImGui::MenuItem("Save"))
        {
            ctx->GetStoragePolicy()->Save(ctx);
            ImGui::CloseCurrentPopup();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Reload"))
        {
            ctx->GetStoragePolicy()->Load(ctx);
            ImGui::CloseCurrentPopup();
        }

        // Switch Level...
        if (ImGui::BeginMenu("Switch Level..."))
        {
            auto* lm = ctx->GetLevelManager();
            const std::string active = lm->GetActiveLevelName();

            for (const auto& levelName : lm->GetLevelNames())
            {
                const bool isActive = (!active.empty() && levelName == active);
                if (ImGui::MenuItem(levelName.c_str(), nullptr, isActive))
                {
                    if (!isActive)
                    {
                        ctx->GetCommandStack()->Execute(
                            std::make_unique<CmdSetActiveLevel>(levelName),
                            ctx
                        );
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Create New Level"))
        {
            OpenCreateLevelModal();
        }

        ImGui::EndPopup();
    }
}

void EngineHeaderPolicy::OpenCreateLevelModal()
{
    m_openCreateModalNext = true;
}

void EngineHeaderPolicy::DrawCreateLevelModal(LevelEditorContext* ctx)
{
    if (m_openCreateModalNext) {
        ImGui::OpenPopup("Create Level");
        m_openCreateModalNext = false;
    }

    bool open = true;
    if (ImGui::BeginPopupModal("Create Level", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Choose a name for the new level:");
        ImGui::Separator();
        ImGui::SetNextItemWidth(320.0f);
        ImGui::InputText("##new_level_name", m_newLevelName, IM_ARRAYSIZE(m_newLevelName));

        ImGui::Separator();
        bool doCreate = ImGui::Button("Create");
        ImGui::SameLine();
        bool doCancel = ImGui::Button("Cancel");

        if (doCreate)
        {
            std::string levelName = std::string(m_newLevelName);
            ctx->GetCommandStack()->Execute(
                std::make_unique<CmdCreateLevel>(levelName),
                ctx
            );
            ImGui::CloseCurrentPopup();
        }
        if (doCancel) 
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

bool EngineHeaderPolicy::DrawLevelIconButton()
{
    // If we don’t have an icon SRV, still render a text-only button in the same shape.
    const bool hasIcon = m_iconLoaded && m_levelIconSRV != nullptr;

    // Layout sizing
    const float pad = 6.0f; // internal padding
    const ImVec2 iconSize = m_cfg.levelIconSize;
    const ImVec2 textSize = ImGui::CalcTextSize(m_cfg.levelLabel ? m_cfg.levelLabel : "Level");
    const float width = std::max(iconSize.x, textSize.x) + pad * 2.0f;
    const float height = iconSize.y + pad + textSize.y + pad * 2.0f;

    // Center this composite button within the current column (nice!)
    const float availX = ImGui::GetContentRegionAvail().x;
    const float cursorX = ImGui::GetCursorPosX();
    const float offsetX = (availX > width) ? (availX - width) * 0.5f : 0.0f;
    ImGui::SetCursorPosX(cursorX + offsetX);

    // The clickable area
    const ImVec2 pMin = ImGui::GetCursorScreenPos();
    const ImVec2 pMax = ImVec2(pMin.x + width, pMin.y + height);

    // Actual button behavior
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const bool pressed = ImGui::InvisibleButton("##level_icon_btn", ImVec2(width, height),
        ImGuiButtonFlags_None);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();

    // Colors
    const ImU32 colBg = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive
        : hovered ? ImGuiCol_ButtonHovered
        : ImGuiCol_Button);
    const ImU32 colBorder = ImGui::GetColorU32(ImGuiCol_Border);
    const ImU32 colText = ImGui::GetColorU32(ImGuiCol_Text);

    // Background & border
    dl->AddRectFilled(pMin, pMax, colBg, 6.0f);
    dl->AddRect(pMin, pMax, colBorder, 6.0f);

    // Icon (centered horizontally, at top)
    float cursorY = pMin.y + pad;
    if (hasIcon)
    {
        const ImVec2 iconPos = ImVec2(pMin.x + (width - iconSize.x) * 0.5f, cursorY);
        const ImVec2 iconMax = ImVec2(iconPos.x + iconSize.x, iconPos.y + iconSize.y);
        dl->AddImage((ImTextureID)m_levelIconSRV, iconPos, iconMax);
        cursorY = iconMax.y + pad;
    }
    else
    {
        cursorY += 2.0f;
    }

    const ImVec2 textPos = ImVec2(pMin.x + (width - textSize.x) * 0.5f, cursorY);
    dl->AddText(textPos, colText, m_cfg.levelLabel ? m_cfg.levelLabel : "Level");

    return pressed;
}
