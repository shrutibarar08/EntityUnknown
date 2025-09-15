#include "ImguiCreationPanel.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include "imgui/imgui_internal.h"

#include "SystemManager/Registry/RegistryLight.h"
#include "Editor/Core/Commands/Commands.h"
#include "Editor/Core/EditorContext.h"


std::string ImguiCreationPanel::lower_copy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

bool ImguiCreationPanel::contains_ic(const std::string& hay, const std::string& needle)
{
    if (needle.empty()) return true;
    auto H = lower_copy(hay);
    auto N = lower_copy(needle);
    return H.find(N) != std::string::npos;
}

std::vector<std::string> ImguiCreationPanel::tokenize(const char* q)
{
    std::vector<std::string> out;
    if (!q) return out;
    const char* p = q;
    while (*p) 
    {
        while (*p && std::isspace((unsigned char)*p)) ++p;
        if (!*p) break;
        const char* b = p;
        while (*p && !std::isspace((unsigned char)*p)) ++p;
        out.emplace_back(b, p - b);
    }
    return out;
}

bool ImguiCreationPanel::Init(LevelEditorContext* context)
{
    ImguiCreationPanel::Item item{};
    item.category = "Lights";
    item.tags.push_back("light");
    item.tags.push_back("illumination");

    for (auto& name : RegistryLight::GetRegisteredNames())
    {
        item.name = name;
        item.onCreate = [&name](LevelEditorContext* context)
        {
            if (!context) return;
            context->GetCommandStack()->Execute
            (
                std::make_unique<CmdCreateLight>(RegistryLight::CreateLight(name)),
                context
            );
        };
        item.featured = true;
        Register(item);
    }
    return true;
}

void ImguiCreationPanel::Register(const Item& it) 
{
    m_items.push_back(it);
}

void ImguiCreationPanel::Register(const std::vector<Item>& items) 
{
    m_items.insert(m_items.end(), items.begin(), items.end());
}

void ImguiCreationPanel::Clear() 
{
    m_items.clear();
    m_recent.clear();
    m_favorites.clear();
    m_selected = -1;
}

void ImguiCreationPanel::DrawCreation(LevelEditorContext* ctx)
{
    if (!ctx) return;

    ImGui::PushID(this);
    ImGui::BeginGroup();
    {
        ImGui::SetNextItemWidth(-120.0f);
        if (ImGui::InputTextWithHint("##create_search", "Search (e.g. 'spot light')", m_search, IM_ARRAYSIZE(m_search))) {
            m_selected = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) 
        {
            m_search[0] = '\0';
            m_selected = -1;
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Show featured", &m_showFeatured)) 
        {
            // TODO: Add Featured
        }

        if (ImGui::BeginPopupContextItem("create_ctx")) 
        {
            ImGui::TextDisabled("Creation Panel Settings");
            ImGui::Separator();
            ImGui::Checkbox("Show featured when idle", &m_showFeatured);
            ImGui::Checkbox("Show categories", &m_showCategories);
            ImGui::Checkbox("Show favorites", &m_showFavorites);
            ImGui::EndPopup();
        }
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const float leftW = m_showCategories ? 160.0f : 0.0f;
    if (m_showCategories) 
    {
        ImGui::BeginChild("##create_categories", ImVec2(leftW, 0), true);
        DrawCategories();
        ImGui::EndChild();
        ImGui::SameLine();
    }

    ImGui::BeginChild("##create_results", ImVec2(0, 0), true);

    HandleKeyboard();

    m_filtered.clear();
    BuildFiltered();

    const bool idle = (std::strlen(m_search) == 0) && (m_activeCategory.empty());

    if (m_showFavorites && !m_favorites.empty() && idle) 
    {
        if (ImGui::CollapsingHeader("★ Favorites", ImGuiTreeNodeFlags_DefaultOpen))
            DrawList(ctx, Section::Favorites);
    }

    if (m_showFeatured && idle) 
    {
        if (ImGui::CollapsingHeader("Featured", ImGuiTreeNodeFlags_DefaultOpen))
            DrawList(ctx, Section::Featured);
    }

    if (ImGui::CollapsingHeader(idle ? "All" : "Results", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawList(ctx, Section::Results);
    }

    if (!m_recent.empty() && idle)
    {
        if (ImGui::CollapsingHeader("Recent", ImGuiTreeNodeFlags_DefaultOpen))
            DrawList(ctx, Section::Recent);
    }

    ImGui::EndChild();
    ImGui::PopID();
}

int ImguiCreationPanel::score_item(const Item& it, const std::vector<std::string>& tokens) const 
{
    if (tokens.empty()) return (it.featured ? 1 : 0); // tiny bias when idle
    int score = 0;
    auto lname = lower_copy(it.name);
    auto lcat = lower_copy(it.category);

    for (auto& t : tokens) 
    {
        auto lt = lower_copy(t);
        bool matched = false;
        // name prefix
        if (lname.rfind(lt, 0) == 0) { score += 3; matched = true; }
        // name contains
        else if (lname.find(lt) != std::string::npos) { score += 2; matched = true; }
        // category or tags
        if (!matched) 
        {
            if (!lcat.empty() && lcat.find(lt) != std::string::npos) { score += 1; matched = true; }
        }
        if (!matched) 
        {
            for (auto& tag : it.tags) 
            {
                if (contains_ic(tag, lt)) { score += 1; break; }
            }
        }
    }
    return score;
}

void ImguiCreationPanel::BuildFiltered() 
{
    const auto tokens = tokenize(m_search);

    std::vector<ScoredIdx> scored;
    scored.reserve(m_items.size());

    for (int i = 0; i < (int)m_items.size(); ++i) 
    {
        const auto& it = m_items[i];

        if (!m_activeCategory.empty()) 
        {
            if (!contains_ic(it.category, m_activeCategory))
                continue;
        }

        const int s = score_item(it, tokens);
        if (!tokens.empty() && s <= 0) continue;

        scored.push_back({ i, s });
    }

    std::sort(scored.begin(), scored.end(), [&](const ScoredIdx& a, const ScoredIdx& b)
    {
        if (a.score != b.score) return a.score > b.score;
        const auto& A = m_items[a.idx];
        const auto& B = m_items[b.idx];
        if (A.featured != B.featured) return A.featured && !B.featured;
        return A.name < B.name;
    });

    m_filtered.clear();
    m_filtered.reserve(scored.size());
    for (auto& s : scored) m_filtered.push_back(s.idx);

    if (m_selected >= (int)m_filtered.size()) m_selected = (int)m_filtered.size() - 1;
}

void ImguiCreationPanel::HandleKeyboard()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) 
        {
            if (m_selected < (int)m_filtered.size() - 1) ++m_selected;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) 
        {
            if (m_selected > 0) --m_selected;
        }
    }
}

void ImguiCreationPanel::DrawList(LevelEditorContext* ctx, Section sec)
{
    ImGui::PushID(static_cast<int>(sec));
    switch (sec)
    {
        case Section::Results:   DrawResults(ctx);   break;
        case Section::Featured:  DrawFeatured(ctx);  break;
        case Section::Favorites: DrawFavorites(ctx); break;
        case Section::Recent:    DrawRecent(ctx);    break;
    }
    ImGui::PopID();
}

void ImguiCreationPanel::DrawResults(LevelEditorContext* ctx)
{
    ImGui::PushID("Results");

    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg));

    if (ImGui::BeginTable("##create_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.6f);
        ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthStretch, 0.25f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor(3);

        for (int row = 0; row < (int)m_filtered.size(); ++row)
        {
            const int idx = m_filtered[row];
            const auto& it = m_items[idx];

            ImGui::PushID(row);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGuiSelectableFlags sflags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;
            const bool selected = (row == m_selected);
            if (ImGui::Selectable(it.name.c_str(), selected, sflags))
            {
                m_selected = row;
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    TriggerCreate(ctx, it);
            }

            // <- crucial: allow later widgets to receive clicks even if they overlap the selectable
            ImGui::SetItemAllowOverlap();

            ImGui::SameLine();
            bool fav = (m_favorites.count(it.name) != 0);
            if (ImGui::SmallButton(fav ? "★##fav" : "☆##fav"))
                ToggleFavorite(it.name);

            if (selected && ImGui::IsKeyPressed(ImGuiKey_Enter))
                TriggerCreate(ctx, it);

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(it.category.empty() ? "-" : it.category.c_str());

            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Create##row"))
            {
                TriggerCreate(ctx, it);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
    else
    {
        ImGui::PopStyleColor(3);
    }

    ImGui::PopID();
}

void ImguiCreationPanel::DrawFeatured(LevelEditorContext* ctx)
{
    for (const auto& it : m_items)
        if (it.featured) DrawCard(ctx, it, Section::Featured);
}

void ImguiCreationPanel::DrawFavorites(LevelEditorContext* ctx)
{
    for (const auto& it : m_items)
        if (m_favorites.count(it.name))
            DrawCard(ctx, it, Section::Favorites);
}

void ImguiCreationPanel::DrawRecent(LevelEditorContext* ctx)
{
    for (const auto& name : m_recent)
        if (auto* it = FindByName(name)) DrawCard(ctx, *it, Section::Recent);
}

void ImguiCreationPanel::DrawCard(LevelEditorContext* ctx, const Item& it, Section sec)
{
    ImGui::PushID(static_cast<int>(sec));
    ImGui::PushID(it.name.c_str());

    ImGui::BeginGroup();
    ImGui::Text("%s", it.name.c_str());
    if (!it.category.empty())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("— %s", it.category.c_str());
    }
    if (ImGui::SmallButton("Create"))
    {
        TriggerCreate(ctx, it);
    }
    ImGui::EndGroup();
    ImGui::Separator();

    ImGui::PopID();
    ImGui::PopID();
}

ImguiCreationPanel::Item* ImguiCreationPanel::FindByName(const std::string& n)
{
    for (auto& it : m_items) if (it.name == n) return &it;
    return nullptr;
}

void ImguiCreationPanel::PushRecent(const std::string& n)
{
    auto it = std::find(m_recent.begin(), m_recent.end(), n);
    if (it != m_recent.end()) m_recent.erase(it);
    m_recent.push_front(n);
    if ((int)m_recent.size() > kMaxRecent) m_recent.pop_back();
}

void ImguiCreationPanel::TriggerCreate(LevelEditorContext* ctx, const Item& it)
{
    if (it.onCreate) it.onCreate(ctx);
    PushRecent(it.name);
}

void ImguiCreationPanel::ToggleFavorite(const std::string& name)
{
    if (m_favorites.count(name)) m_favorites.erase(name);
    else m_favorites.insert(name);
}

void ImguiCreationPanel::DrawCategories()
{
    std::vector<std::string> cats;
    cats.reserve(16);
    cats.emplace_back("(All)");
    for (auto& it : m_items) 
    {
        if (it.category.empty()) continue;
        if (std::find(cats.begin(), cats.end(), it.category) == cats.end())
            cats.push_back(it.category);
    }
    std::sort(cats.begin() + 1, cats.end());

    if (ImGui::Selectable("(All)", m_activeCategory.empty()))
        m_activeCategory.clear();

    for (auto& c : cats) 
    {
        if (c == "(All)") continue;
        bool sel = (m_activeCategory == c);
        if (ImGui::Selectable(c.c_str(), sel))
        {
            m_activeCategory = c;
            m_selected = 0;
        }
    }
}
