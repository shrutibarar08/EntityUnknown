#include "ImguiCreationPanelHorizontal.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include "imgui/imgui_internal.h"

// ---------------- Utils ----------------

std::string ImguiCreationPanelHorizontal::lower_copy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

bool ImguiCreationPanelHorizontal::contains_ic(const std::string& hay, const std::string& needle) {
    if (needle.empty()) return true;
    auto H = lower_copy(hay);
    auto N = lower_copy(needle);
    return H.find(N) != std::string::npos;
}

std::vector<std::string> ImguiCreationPanelHorizontal::tokenize(const char* q) {
    std::vector<std::string> out;
    if (!q) return out;
    const char* p = q;
    while (*p) {
        while (*p && std::isspace((unsigned char)*p)) ++p;
        if (!*p) break;
        const char* b = p;
        while (*p && !std::isspace((unsigned char)*p)) ++p;
        out.emplace_back(b, p - b);
    }
    return out;
}

// ---------------- Public API ----------------

void ImguiCreationPanelHorizontal::Register(const Item& it) {
    m_items.push_back(it);
}

void ImguiCreationPanelHorizontal::Register(const std::vector<Item>& items) {
    m_items.insert(m_items.end(), items.begin(), items.end());
}

void ImguiCreationPanelHorizontal::Clear() {
    m_items.clear();
    m_filtered.clear();
    m_recent.clear();
    m_favorites.clear();
    m_selected = -1;
}

// ---------------- Draw ----------------

void ImguiCreationPanelHorizontal::DrawCreation(LevelEditorContext* ctx) {
    if (!ctx) return;

    ImGui::PushID(this);

    // Top bar (search + settings aligned horizontally)
    DrawTopBar();

    // Category/Tab strip: horizontal
    DrawCategoryTabs();

    // Results area
    ImGui::BeginChild("##create_h_grid", ImVec2(0, 0), true);
    BuildFiltered();
    HandleKeyboardGrid();
    DrawGrid(ctx);
    ImGui::EndChild();

    ImGui::PopID();
}

// ---------------- Internals ----------------

int ImguiCreationPanelHorizontal::score_item(const Item& it, const std::vector<std::string>& tokens) const {
    if (tokens.empty()) return (it.featured ? 1 : 0);
    int score = 0;
    const auto lname = lower_copy(it.name);
    const auto lcat = lower_copy(it.category);

    for (auto& t : tokens) {
        const auto lt = lower_copy(t);
        bool matched = false;
        if (lname.rfind(lt, 0) == 0) { score += 3; matched = true; } // prefix
        else if (lname.find(lt) != std::string::npos) { score += 2; matched = true; } // substring
        if (!matched && !lcat.empty() && lcat.find(lt) != std::string::npos) { score += 1; matched = true; }
        if (!matched) {
            for (auto& tag : it.tags) {
                if (contains_ic(tag, lt)) { score += 1; break; }
            }
        }
    }
    return score;
}

void ImguiCreationPanelHorizontal::BuildFiltered() {
    m_filtered.clear();

    // Build base list given active tab/category
    const bool hasQuery = std::strlen(m_search) > 0;
    const auto tokens = tokenize(m_search);

    std::vector<ScoredIdx> scored;
    scored.reserve(m_items.size());

    auto include_by_tab = [&](const Item& it)->bool {
        switch (m_activeTab) {
        case Tab::All:       return true;
        case Tab::Favorites: return m_favorites.count(it.name) != 0;
        case Tab::Featured:  return it.featured;
        case Tab::Recent:    return std::find(m_recent.begin(), m_recent.end(), it.name) != m_recent.end();
        }
        return true;
        };

    for (int i = 0; i < (int)m_items.size(); ++i) {
        const auto& it = m_items[i];

        if (!include_by_tab(it)) continue;

        if (!m_activeCategory.empty()) {
            if (!contains_ic(it.category, m_activeCategory))
                continue;
        }

        const int s = score_item(it, tokens);
        if (hasQuery && s <= 0) continue;

        scored.push_back({ i, s });
    }

    // Sort for horizontal browsing: score desc, then featured, then name asc
    std::sort(scored.begin(), scored.end(), [&](const ScoredIdx& a, const ScoredIdx& b) {
        if (a.score != b.score) return a.score > b.score;
        const auto& A = m_items[a.idx];
        const auto& B = m_items[b.idx];
        if (A.featured != B.featured) return A.featured && !B.featured;
        return A.name < B.name;
        });

    for (auto& s : scored) m_filtered.push_back(s.idx);

    if (m_selected >= (int)m_filtered.size()) m_selected = (int)m_filtered.size() - 1;
}

void ImguiCreationPanelHorizontal::HandleKeyboardGrid() {
    // grid navigation with arrow keys (uses m_columns)
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) return;
    const int total = (int)m_filtered.size();
    if (total == 0) { m_selected = -1; return; }

    auto clampSel = [&](int v) { return std::clamp(v, 0, total - 1); };

    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        m_selected = clampSel((m_selected < 0) ? 0 : m_selected + 1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        m_selected = clampSel((m_selected < 0) ? 0 : m_selected - 1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        m_selected = clampSel((m_selected < 0) ? 0 : m_selected + m_columns);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        m_selected = clampSel((m_selected < 0) ? 0 : m_selected - m_columns);
    }
    if (m_selected >= 0 && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        const int idx = m_filtered[m_selected];
        TriggerCreate(nullptr, m_items[idx]); // ctx passed in DrawGrid per-card; we just mark here (no-op if null)
    }
}

void ImguiCreationPanelHorizontal::DrawTopBar() {
    // search box + columns + toggles laid out horizontally
    if (ImGui::BeginTable("##create_h_top", 6, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(260.0f);
        ImGui::InputTextWithHint("##search", "Search (e.g. 'spot light')", m_search, IM_ARRAYSIZE(m_search));

        ImGui::TableNextColumn();
        if (ImGui::Button("Clear")) { m_search[0] = '\0'; m_selected = -1; }

        ImGui::TableNextColumn();
        ImGui::TextUnformatted("|");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderInt("Columns", &m_columns, 2, 8);

        ImGui::TableNextColumn();
        ImGui::Checkbox("Favorites", &m_showFavorites);

        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Featured", &m_showFeatured)) {
            if (m_activeTab == Tab::Featured && !m_showFeatured) m_activeTab = Tab::All;
        }

        ImGui::EndTable();
    }
    ImGui::Separator();
}

void ImguiCreationPanelHorizontal::DrawCategoryTabs() {
    // Primary tab bar (All/Favorites/Featured/Recent)
    if (ImGui::BeginTabBar("##create_h_tabs", ImGuiTabBarFlags_Reorderable)) {
        if (ImGui::BeginTabItem("All", nullptr, (m_activeTab == Tab::All ? ImGuiTabItemFlags_SetSelected : 0))) {
            m_activeTab = Tab::All; ImGui::EndTabItem();
        }
        if (m_showFavorites && ImGui::BeginTabItem("★ Favorites", nullptr, (m_activeTab == Tab::Favorites ? ImGuiTabItemFlags_SetSelected : 0))) {
            m_activeTab = Tab::Favorites; ImGui::EndTabItem();
        }
        if (m_showFeatured && ImGui::BeginTabItem("Featured", nullptr, (m_activeTab == Tab::Featured ? ImGuiTabItemFlags_SetSelected : 0))) {
            m_activeTab = Tab::Featured; ImGui::EndTabItem();
        }
        if (m_showRecent && ImGui::BeginTabItem("Recent", nullptr, (m_activeTab == Tab::Recent ? ImGuiTabItemFlags_SetSelected : 0))) {
            m_activeTab = Tab::Recent; ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // Secondary: category buttons as a horizontal wrap
    ImGui::BeginChild("##cat_strip", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 1.8f), false, ImGuiWindowFlags_NoScrollbar);
    // Collect categories
    std::vector<std::string> cats;
    cats.reserve(16);
    for (auto& it : m_items)
        if (!it.category.empty())
            if (std::find(cats.begin(), cats.end(), it.category) == cats.end())
                cats.push_back(it.category);
    std::sort(cats.begin(), cats.end());

    // "(All categories)" button
    bool selAll = m_activeCategory.empty();
    if (ImGui::Selectable("All Categories", selAll, ImGuiSelectableFlags_AllowDoubleClick)) {
        m_activeCategory.clear();
    }
    ImGui::SameLine();
    for (size_t i = 0; i < cats.size(); ++i) {
        bool sel = (m_activeCategory == cats[i]);
        if (ImGui::Selectable(cats[i].c_str(), sel, ImGuiSelectableFlags_AllowDoubleClick)) {
            m_activeCategory = cats[i];
        }
        if (i + 1 < cats.size()) ImGui::SameLine();
    }
    ImGui::EndChild();
    ImGui::Separator();
}

void ImguiCreationPanelHorizontal::DrawGrid(LevelEditorContext* ctx) {
    const int total = (int)m_filtered.size();
    if (total == 0) {
        ImGui::TextDisabled("No results.");
        return;
    }
    if (m_columns < 1) m_columns = 1;

    // Use ImGui tables to build a grid
    if (ImGui::BeginTable("##create_h_grid_table", m_columns,
        ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
    {
        int col = 0;
        for (int i = 0; i < total; ++i) {
            if (col == 0) ImGui::TableNextRow();

            ImGui::TableNextColumn();
            const int itemIdx = m_filtered[i];
            bool selected = (i == m_selected);

            // Draw a "card"
            DrawCard(ctx, m_items[itemIdx], selected);

            col = (col + 1) % m_columns;
        }
        ImGui::EndTable();
    }
}

void ImguiCreationPanelHorizontal::DrawCard(LevelEditorContext* ctx, const Item& it, bool selected) {
    ImGui::PushID(it.name.c_str());

    // Visual: framed group
    ImGui::BeginGroup();

    if (selected) {
        // Subtle selection highlight
        auto col = ImGui::GetStyleColorVec4(ImGuiCol_Header);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(col.x, col.y, col.z, 0.15f));
        ImGui::BeginChild("##sel", ImVec2(-1, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    }
    else {
        ImGui::BeginChild("##card", ImVec2(-1, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    }

    ImGui::TextUnformatted(it.name.c_str());
    if (!it.category.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("— %s", it.category.c_str());
    }

    // Tag row (compact)
    if (!it.tags.empty()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
        for (size_t i = 0; i < it.tags.size(); ++i) {
            ImGui::SmallButton(it.tags[i].c_str());
            if (i + 1 < it.tags.size()) ImGui::SameLine();
        }
        ImGui::PopStyleVar();
    }

    // Action row
    bool isFav = (m_favorites.count(it.name) != 0);
    if (ImGui::SmallButton("Create")) {
        TriggerCreate(ctx, it);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(isFav ? "★" : "☆")) {
        ToggleFavorite(it.name);
    }

    // Double-click anywhere inside card to create
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        TriggerCreate(ctx, it);
    }

    ImGui::EndChild();

    if (selected) ImGui::PopStyleColor();

    ImGui::EndGroup();
    ImGui::PopID();
}

void ImguiCreationPanelHorizontal::PushRecent(const std::string& n) {
    auto it = std::find(m_recent.begin(), m_recent.end(), n);
    if (it != m_recent.end()) m_recent.erase(it);
    m_recent.push_front(n);
    if ((int)m_recent.size() > kMaxRecent) m_recent.pop_back();
}

void ImguiCreationPanelHorizontal::TriggerCreate(LevelEditorContext* ctx, const Item& it) {
    if (it.onCreate && ctx) it.onCreate(*ctx);
    PushRecent(it.name);
}

ImguiCreationPanelHorizontal::Item* ImguiCreationPanelHorizontal::FindByName(const std::string& n) {
    for (auto& it : m_items) if (it.name == n) return &it;
    return nullptr;
}

void ImguiCreationPanelHorizontal::ToggleFavorite(const std::string& name) {
    if (m_favorites.count(name)) m_favorites.erase(name);
    else m_favorites.insert(name);
}
