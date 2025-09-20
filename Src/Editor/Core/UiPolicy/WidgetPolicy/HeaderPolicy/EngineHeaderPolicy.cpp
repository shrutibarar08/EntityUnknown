#include "EngineHeaderPolicy.h"
#include "imgui/imgui_internal.h"
#include "Editor/Core/EditorContext.h"
#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"

#include "Editor/EditorState.h"

bool EngineHeaderPolicy::Init(LevelEditorContext* context)
{
    if (m_cfg.levelIconPath)
    {
        auto t = TextureLoader::GetTexture(m_cfg.levelIconPath);
        m_levelIconSRV = t.ShaderResourceView;
        m_iconLoaded = t.IsInitialized();
    }
    else
    {
        m_iconLoaded = false;
        m_levelIconSRV = nullptr;
    }

    if (m_cfg.playIconPath)
    {
        auto t = TextureLoader::GetTexture(m_cfg.playIconPath);
        m_playIconSRV = t.ShaderResourceView;
        m_playIconLoaded = t.IsInitialized();
    }

    if (m_cfg.stopIconPath)
    {
        auto t = TextureLoader::GetTexture(m_cfg.stopIconPath);
        m_stopIconSRV = t.ShaderResourceView;
        m_stopIconLoaded = t.IsInitialized();
    }

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
        ImGuiWindowFlags_NoResize);
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

    ImGui::BeginGroup();

    ImGui::PushID("play_btn");
    const bool playPressed = DrawIconButton(
        "##play",
        m_playIconSRV,
        m_cfg.playIconSize.x > 0 ? m_cfg.playIconSize : ImVec2(18, 18),
        "Play",
        m_cfg.playAccent
    );
    if (playPressed)
        EDITOR_STATE::PLAY_STATE = true;
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Play (set PLAY_STATE = true)");
    ImGui::PopID();

    ImGui::SameLine(0.0f, 6.0f);

    ImGui::PushID("stop_btn");
    const bool stopPressed = DrawIconButton(
        "##stop",
        m_stopIconSRV,
        m_cfg.stopIconSize.x > 0 ? m_cfg.stopIconSize : ImVec2(18, 18),
        "Stop",
        m_cfg.stopAccent
    );
    if (stopPressed)
        EDITOR_STATE::PLAY_STATE = false;
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Stop (set PLAY_STATE = false)");
    ImGui::PopID();

    ImGui::SameLine(0.0f, 12.0f);

    ImGui::TextDisabled("Status: %s", EDITOR_STATE::PLAY_STATE ? "Playing" : "Ready");

    ImGui::EndGroup();
}

void EngineHeaderPolicy::DrawLevelMenu(LevelEditorContext* ctx)
{
    if (!ctx) { return; }

    if (ImGui::BeginPopup("##level_popup"))
    {
        if (ImGui::MenuItem("Save"))
        {
            if (auto* sp = ctx->GetStoragePolicy()) { sp->Save(ctx); }
            ImGui::CloseCurrentPopup();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Reload"))
        {
            if (auto* sp = ctx->GetStoragePolicy()) { sp->Load(ctx); }
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::BeginMenu("Switch Level"))
        {
            if (auto* lm = ctx->GetLevelManager())
            {
                const std::string active = lm->GetActiveLevelName();
                for (const auto& levelName : lm->GetLevelNames())
                {
                    const bool isActive = (!active.empty() && levelName == active);
                    if (ImGui::MenuItem(levelName.c_str(), nullptr, isActive /*show check*/))
                    {
                        if (!isActive)
                        {
                            if (auto* cs = ctx->GetCommandStack())
                            {
                                cs->Execute(std::make_unique<CmdSetActiveLevel>(levelName), ctx);
                            }
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Delete Level"))
        {
            if (auto* lm = ctx->GetLevelManager())
            {
                for (const auto& levelName : lm->GetLevelNames())
                {
                    if (levelName.empty()) continue;
                    if (ImGui::MenuItem(levelName.c_str()))
                    {
                        if (auto* cs = ctx->GetCommandStack())
                        {
                            cs->Execute(std::make_unique<CmdDeleteLevel>(levelName), ctx);
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndMenu();
        }

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

bool EngineHeaderPolicy::DrawIconButton(const char* id,
    ID3D11ShaderResourceView* srv,
    ImVec2 size,
    const char* fallbackText,
    ImU32 accent)
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float pad = (m_cfg.iconPad > 0.0f ? m_cfg.iconPad : 4.0f);
    const ImVec2 btnSize(size.x + pad * 2.0f, size.y + pad * 2.0f);

    const ImVec2 pMin = ImGui::GetCursorScreenPos();
    const ImVec2 pMax = ImVec2(pMin.x + btnSize.x, pMin.y + btnSize.y);

    const bool pressed = ImGui::InvisibleButton(id, btnSize, ImGuiButtonFlags_None);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();

    ImU32 colBg = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive :
        hovered ? ImGuiCol_ButtonHovered :
        ImGuiCol_Button);

    if (accent != 0)
    {
        ImVec4 bg = ImGui::ColorConvertU32ToFloat4(colBg);
        ImVec4 ac = ImGui::ColorConvertU32ToFloat4(accent);
        const float w = 0.25f; // influence
        ImVec4 out{ bg.x * (1 - w) + ac.x * w, bg.y * (1 - w) + ac.y * w, bg.z * (1 - w) + ac.z * w, bg.w };
        colBg = ImGui::ColorConvertFloat4ToU32(out);
    }

    const ImU32 colBorder = ImGui::GetColorU32(ImGuiCol_Border);

    dl->AddRectFilled(pMin, pMax, colBg, 6.0f);
    dl->AddRect(pMin, pMax, colBorder, 6.0f);

    const ImVec2 imgMin = ImVec2(pMin.x + (btnSize.x - size.x) * 0.5f,
        pMin.y + (btnSize.y - size.y) * 0.5f);
    const ImVec2 imgMax = ImVec2(imgMin.x + size.x, imgMin.y + size.y);

    if (srv)
        dl->AddImage((ImTextureID)srv, imgMin, imgMax);
    else
        dl->AddText(ImVec2(pMin.x + pad, pMin.y + pad), ImGui::GetColorU32(ImGuiCol_Text), fallbackText);

    return pressed;
}
