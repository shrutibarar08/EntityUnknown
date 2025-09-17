#include "ImguiCreationPanel.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include "imgui/imgui_internal.h"

#include "Editor/Core/Commands/Commands.h"
#include "Editor/Core/EditorContext.h"

#include "RenderManager/Light/DefineLights.h"
#include "RenderManager/DefineRenders.h"

#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"


static ID3D11ShaderResourceView* TryGetIcon(
    const ImguiCreationPanel::Config& cfg,
    const char* key)
{
    auto it = cfg.iconPaths.find(key);
    if (it == cfg.iconPaths.end()) 
    {
        return nullptr;
    }

    auto tex = TextureLoader::GetTexture(it->second);
    return tex.ShaderResourceView;
}

namespace
{
    constexpr ImVec2 kIcon16{ 16,16 };
    constexpr ImVec2 kIcon20{ 20,20 };

    inline bool SmallButtonWithId(const char* id, const char* label)
    {
        std::string text = std::string(label) + "##" + id;
        return ImGui::SmallButton(text.c_str());
    }

    inline bool IconButton(const char* id,
        ID3D11ShaderResourceView* srv,
        ImVec2 size,
        const char* fallback)
    {
        if (srv != nullptr)
        {
            return ImGui::ImageButton(id, (ImTextureID)srv, size);
        }
        return SmallButtonWithId(id, fallback);
    }

    inline bool RightAlignedIconButton(const char* id,
        ID3D11ShaderResourceView* srv,
        ImVec2 size,
        const char* fallback)
    {
        ImGuiStyle& st = ImGui::GetStyle();
        float w = (srv != nullptr)
            ? size.x + st.FramePadding.x * 2.0f
            : ImGui::CalcTextSize(fallback).x + st.FramePadding.x * 2.0f;

        float avail = ImGui::GetContentRegionAvail().x;
        if (avail < w + 2.0f)
        {
            ImGui::NewLine();
            avail = ImGui::GetContentRegionAvail().x;
        }

        float x = ImGui::GetCursorPosX() + avail - w;
        ImGui::SameLine();
        ImGui::SetCursorPosX(x);

        return IconButton(id, srv, size, fallback);
    }

    inline void RowIcon(ID3D11ShaderResourceView* srv)
    {
        if (srv != nullptr)
        {
            ImGui::Image((ImTextureID)srv, kIcon16);
            ImGui::SameLine(0.0f, 6.0f);
        }
    }
}

std::string ImguiCreationPanel::LowerCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

bool ImguiCreationPanel::ContainsIC(const std::string& hay, const std::string& needle)
{
    if (needle.empty())
    {
        return true;
    }
    auto H = LowerCopy(hay);
    auto N = LowerCopy(needle);
    return H.find(N) != std::string::npos;
}

std::vector<std::string> ImguiCreationPanel::Tokenize(const char* q)
{
    std::vector<std::string> out;
    if (!q)
    {
        return out;
    }
    const char* p = q;
    while (*p)
    {
        while (*p && std::isspace((unsigned char)*p)) { ++p; }
        if (!*p) { break; }
        const char* b = p;
        while (*p && !std::isspace((unsigned char)*p)) { ++p; }
        out.emplace_back(b, p - b);
    }
    return out;
}

bool ImguiCreationPanel::Init(LevelEditorContext* context)
{
    using Item = ImguiCreationPanel::Item;

    ImguiCreationPanel::Config cfg{};
    cfg.iconPaths["icon.light"]         = "Assets/UI/icons/light.png";
    cfg.iconPaths["icon.mesh"]          = "Assets/UI/icons/mesh.png";
    cfg.iconPaths["icon.sprite"]        = "Assets/UI/icons/sprite.png";
    cfg.iconPaths["icon.favorite_on"]   = "Assets/UI/icons/star_on.png";
    cfg.iconPaths["icon.favorite_off"]  = "Assets/UI/icons/star_off.png";
    cfg.iconPaths["icon.create"]        = "Assets/UI/icons/plus.png";
    cfg.iconPaths["icon.clear"]         = "Assets/UI/icons/clear.png";

    SetConfig(cfg);

    LoadIconsIfNeeded();

    { // Lights
        Item item{};
        item.category = "Lights";
        item.tags = { "light", "illumination" };

        for (const auto& name : RegistryLight::GetRegisteredNames())
        {
            item.name = name;
            item.onCreate = [name](LevelEditorContext* ctx)
                {
                    if (!ctx) { return; }
                    auto* stack = ctx->GetCommandStack();
                    if (!stack) { return; }
                    stack->Execute(
                        std::make_unique<CmdCreateLight>(RegistryLight::CreateLight(name)),
                        ctx
                    );
                };
            item.featured = false;
            Register(item);
        }
    }

    { // Meshes (3D)
        Item item{};
        item.category = "Mesh";
        item.tags = { "mesh", "object", "3D" };

        for (const auto& name : RegistryMesh::GetRegisteredNames())
        {
            if (name.ends_with("Sprite"))
            {
                continue;
            }
            item.name = name;
            item.onCreate = [name](LevelEditorContext* ctx)
                {
                    if (!ctx) { return; }
                    auto* stack = ctx->GetCommandStack();
                    if (!stack) { return; }
                    stack->Execute(
                        std::make_unique<CmdCreateMesh>(name),
                        ctx
                    );
                };
            item.featured = false;
            Register(item);
        }
    }

    { // Sprites: Background (2D)
        Item item{};
        item.category = "Sprite";
        item.tags = { "sprite", "2D", "image", "background" };

        for (const auto& name : RegistryMesh::GetRegisteredNames())
        {
            if (name != "BackgroundSprite")
            {
                continue;
            }
            item.name = name;
            item.onCreate = [name](LevelEditorContext* ctx)
                {
                    if (!ctx) { return; }
                    auto* stack = ctx->GetCommandStack();
                    if (!stack) { return; }
                    stack->Execute(
                        std::make_unique<CmdCreateBackgroundSprite>(name),
                        ctx
                    );
                };
            item.featured = false;
            Register(item);
        }
    }

    { // Sprites: Front (2D)
        Item item{};
        item.category = "Sprite";
        item.tags = { "sprite", "2D", "image", "front" };

        for (const auto& name : RegistryMesh::GetRegisteredNames())
        {
            if (name != "ScreenSprite")
            {
                continue;
            }
            item.name = name;
            item.onCreate = [name](LevelEditorContext* ctx)
                {
                    if (!ctx) { return; }
                    auto* stack = ctx->GetCommandStack();
                    if (!stack) { return; }
                    stack->Execute(
                        std::make_unique<CmdCreateFrontSprite>(name),
                        ctx
                    );
                };
            item.featured = false;
            Register(item);
        }
    }

    { // Sprites: World Space
        Item item{};
        item.category = "Sprite";
        item.tags = { "sprite", "2D", "image", "world space", "3D Sprite" };

        for (const auto& name : RegistryMesh::GetRegisteredNames())
        {
            if (name != "WorldSpaceSprite")
            {
                continue;
            }
            item.name = name;
            item.onCreate = [name](LevelEditorContext* ctx)
                {
                    if (!ctx) { return; }
                    auto* stack = ctx->GetCommandStack();
                    if (!stack) { return; }
                    stack->Execute(
                        std::make_unique<CmdCreateMesh>(name),
                        ctx
                    );
                };
            item.featured = false;
            Register(item);
        }
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

// --------------------- scoring & filter ------------------
int ImguiCreationPanel::ScoreItem(const Item& it, const std::vector<std::string>& tokens) const
{
    if (tokens.empty())
    {
        return (it.featured ? 1 : 0);
    }

    int score = 0;
    auto lname = LowerCopy(it.name);
    auto lcat = LowerCopy(it.category);

    for (auto& t : tokens)
    {
        auto lt = LowerCopy(t);
        bool matched{ false };

        if (lname.rfind(lt, 0) == 0)
        {
            score += 3;
            matched = true;
        }
        else if (lname.find(lt) != std::string::npos)
        {
            score += 2;
            matched = true;
        }

        if (!matched && !lcat.empty() && lcat.find(lt) != std::string::npos)
        {
            score += 1;
            matched = true;
        }

        if (!matched)
        {
            for (auto& tag : it.tags)
            {
                if (ContainsIC(tag, lt))
                {
                    score += 1;
                    break;
                }
            }
        }
    }
    return score;
}

void ImguiCreationPanel::BuildFiltered()
{
    const auto tokens = Tokenize(m_search);

    std::vector<ScoredIdx> scored;
    scored.reserve(m_items.size());

    for (int i = 0; i < (int)m_items.size(); ++i)
    {
        const auto& it = m_items[i];

        if (!m_activeCategory.empty())
        {
            if (!ContainsIC(it.category, m_activeCategory))
            {
                continue;
            }
        }

        const int s = ScoreItem(it, tokens);
        if (!tokens.empty() && s <= 0)
        {
            continue;
        }

        scored.push_back({ i, s });
    }

    std::sort(scored.begin(), scored.end(), [&](const ScoredIdx& a, const ScoredIdx& b)
        {
            if (a.score != b.score)
            {
                return a.score > b.score;
            }
            const auto& A = m_items[a.idx];
            const auto& B = m_items[b.idx];
            if (A.featured != B.featured)
            {
                return A.featured && !B.featured;
            }
            return A.name < B.name;
        });

    m_filtered.clear();
    m_filtered.reserve(scored.size());
    for (auto& s : scored)
    {
        m_filtered.push_back(s.idx);
    }

    if (m_selected >= (int)m_filtered.size())
    {
        m_selected = (int)m_filtered.size() - 1;
    }
}

void ImguiCreationPanel::HandleKeyboard()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        {
            if (m_selected < (int)m_filtered.size() - 1) { ++m_selected; }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        {
            if (m_selected > 0) { --m_selected; }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            if (m_selected >= 0 && m_selected < (int)m_filtered.size())
            {
                const auto& it = m_items[m_filtered[m_selected]];
                TriggerCreate(nullptr, it); // will no-op if ctx is needed (we pass real ctx elsewhere)
            }
        }
    }
}

// --------------------- UI: toolbar -----------------------
void ImguiCreationPanel::DrawToolbarLeft(ImguiCreationPanel* self)
{
    ImGui::SetNextItemWidth(-120.0f);
    if (ImGui::InputTextWithHint("##create_search", "Search…", self->m_search, IM_ARRAYSIZE(self->m_search)))
    {
        self->m_selected = 0;
    }

    ImGui::SameLine();
    if (IconButton("clear", self->m_icons.clear, kIcon20, "Clear"))
    {
        self->m_search[0] = '\0';
        self->m_selected = -1;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Show featured", &self->m_showFeatured);
}

void ImguiCreationPanel::LoadIconsIfNeeded()
{
    if (m_icons.ready)
    {
        return;
    }

    m_icons.light = TryGetIcon(m_config, "icon.light");
    m_icons.mesh = TryGetIcon(m_config, "icon.mesh");
    m_icons.sprite = TryGetIcon(m_config, "icon.sprite");
    m_icons.favOn = TryGetIcon(m_config, "icon.favorite_on");
    m_icons.favOff = TryGetIcon(m_config, "icon.favorite_off");
    m_icons.create = TryGetIcon(m_config, "icon.create");
    m_icons.clear = TryGetIcon(m_config, "icon.clear");

    m_icons.ready = true;
}

// --------------------- UI: main draw ---------------------
void ImguiCreationPanel::DrawCreation(LevelEditorContext* ctx)
{
    if (!ctx)
    {
        return;
    }

    ImGui::PushID(this);

    ImGui::BeginGroup();
    {
        DrawToolbarLeft(this);
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const float leftW = m_showCategories ? 160.0f : 0.0f;
    if (m_showCategories)
    {
        ImGui::BeginChild("##create_categories", ImVec2(leftW, 0), true);
        {
            DrawCategories();
        }
        ImGui::EndChild();
        ImGui::SameLine();
    }

    ImGui::BeginChild("##create_results", ImVec2(0, 0), true);
    {
        HandleKeyboard();
        m_filtered.clear();
        BuildFiltered();

        const bool idle = (std::strlen(m_search) == 0) && m_activeCategory.empty();

        if (m_showFavorites && !m_favorites.empty() && idle)
        {
            if (ImGui::CollapsingHeader("Favorites", ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawList(ctx, Section::Favorites);
            }
        }
        if (m_showFeatured && idle)
        {
            if (ImGui::CollapsingHeader("Featured", ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawList(ctx, Section::Featured);
            }
        }
        if (ImGui::CollapsingHeader(idle ? "All" : "Results", ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawList(ctx, Section::Results);
        }
        if (!m_recent.empty() && idle)
        {
            if (ImGui::CollapsingHeader("Recent", ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawList(ctx, Section::Recent);
            }
        }
    }
    ImGui::EndChild();

    ImGui::PopID();
}

// --------------------- UI: lists -------------------------
void ImguiCreationPanel::DrawList(LevelEditorContext* ctx, Section sec)
{
    ImGui::PushID(static_cast<int>(sec));
    {
        switch (sec)
        {
        case Section::Results:   DrawResults(ctx);   break;
        case Section::Featured:  DrawFeatured(ctx);  break;
        case Section::Favorites: DrawFavorites(ctx); break;
        case Section::Recent:    DrawRecent(ctx);    break;
        }
    }
    ImGui::PopID();
}

static void BeginResultsTable()
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg));

    if (ImGui::BeginTable("##create_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch, 0.65f);
        ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthStretch, 0.25f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 88.0f);
        ImGui::TableHeadersRow();
    }
}

static void EndResultsTable()
{
    if (ImGui::GetCurrentTable())
    {
        ImGui::EndTable();
    }
    ImGui::PopStyleColor(3);
}

void ImguiCreationPanel::DrawResults(LevelEditorContext* ctx)
{
    constexpr bool kCreateOnLeft = false;

    BeginResultsTable();

    for (int row = 0; row < (int)m_filtered.size(); ++row)
    {
        const int idx = m_filtered[row];
        const auto& it = m_items[idx];

        ImGui::PushID(row);
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            {
                bool fav = (m_favorites.count(it.name) != 0);
                ID3D11ShaderResourceView* favIcon = fav ? m_icons.favOn : m_icons.favOff;
                if (IconButton("fav", favIcon, kIcon16, fav ? "Unfav" : "Fav"))
                {
                    ToggleFavorite(it.name);
                }
                ImGui::SameLine(0.0f, 6.0f);

                ID3D11ShaderResourceView* catIcon =
                    (it.category == "Lights") ? m_icons.light :
                    (it.category == "Mesh") ? m_icons.mesh :
                    (it.category == "Sprite") ? m_icons.sprite : nullptr;

                RowIcon(catIcon);

                if (kCreateOnLeft)
                {
                    if (IconButton("create_left", m_icons.create, kIcon16, "Create"))
                    {
                        TriggerCreate(ctx, it);
                    }
                    ImGui::SameLine(0.0f, 6.0f);
                }

                ImGuiSelectableFlags sflags = ImGuiSelectableFlags_AllowDoubleClick;
                const bool selected = (row == m_selected);
                if (ImGui::Selectable(it.name.c_str(), selected, sflags, ImVec2(0, 0)))
                {
                    m_selected = row;
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        TriggerCreate(ctx, it);
                    }
                }

                if (selected && ImGui::IsKeyPressed(ImGuiKey_Enter))
                {
                    TriggerCreate(ctx, it);
                }
            }

            ImGui::TableNextColumn();
            {
                const char* cat = it.category.empty() ? "-" : it.category.c_str();
                ImVec2 textPos = ImGui::GetCursorScreenPos();
                ImGui::TextUnformatted(cat);

                // make only the text area a dbl-click target (no overlap with other cells)
                ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                ImGui::SetCursorScreenPos(rect.Min);
                std::string cid = std::string("##cat_sel_") + std::to_string(row);
                bool catClick = ImGui::Selectable(cid.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, rect.GetSize());
                if (catClick && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    TriggerCreate(ctx, it);
                }
                ImGui::SetCursorScreenPos(rect.Max);
            }

            if (!kCreateOnLeft)
            {
                ImGui::TableNextColumn();
                {
                    if (RightAlignedIconButton("create", m_icons.create, kIcon20, "Create"))
                    {
                        TriggerCreate(ctx, it);
                    }
                }
            }
            else
            {
                ImGui::TableNextColumn();
            }

            if (ImGui::BeginPopupContextItem("row_ctx"))
            {
                if (ImGui::MenuItem("Create"))
                {
                    TriggerCreate(ctx, it);
                }
                bool isFav = m_favorites.count(it.name);
                if (ImGui::MenuItem(isFav ? "Unfavorite" : "Favorite"))
                {
                    ToggleFavorite(it.name);
                }
                ImGui::EndPopup();
            }
        }
        ImGui::PopID();
    }

    EndResultsTable();
}

void ImguiCreationPanel::DrawFeatured(LevelEditorContext* ctx)
{
    for (const auto& it : m_items)
    {
        if (it.featured)
        {
            DrawCard(ctx, it, Section::Featured);
        }
    }
}

void ImguiCreationPanel::DrawFavorites(LevelEditorContext* ctx)
{
    for (const auto& it : m_items)
    {
        if (m_favorites.count(it.name))
        {
            DrawCard(ctx, it, Section::Favorites);
        }
    }
}

void ImguiCreationPanel::DrawRecent(LevelEditorContext* ctx)
{
    for (const auto& name : m_recent)
    {
        if (auto* it = FindByName(name))
        {
            DrawCard(ctx, *it, Section::Recent);
        }
    }
}

void ImguiCreationPanel::DrawCard(LevelEditorContext* ctx, const Item& it, Section sec)
{
    ImGui::PushID(static_cast<int>(sec));
    ImGui::PushID(it.name.c_str());
    {
        ImGui::BeginGroup();
        {
            ID3D11ShaderResourceView* catIcon =
                (it.category == "Lights") ? m_icons.light :
                (it.category == "Mesh") ? m_icons.mesh :
                (it.category == "Sprite") ? m_icons.sprite : nullptr;

            RowIcon(catIcon);
            ImGui::TextUnformatted(it.name.c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("%s", it.category.empty() ? "-" : it.category.c_str());

            if (RightAlignedIconButton("create_card", m_icons.create, kIcon20, "Create"))
            {
                TriggerCreate(ctx, it);
            }
        }
        ImGui::EndGroup();
        ImGui::Separator();
    }
    ImGui::PopID();
    ImGui::PopID();
}

// --------------------- data ops --------------------------
ImguiCreationPanel::Item* ImguiCreationPanel::FindByName(const std::string& n)
{
    for (auto& it : m_items)
    {
        if (it.name == n)
        {
            return &it;
        }
    }
    return nullptr;
}

void ImguiCreationPanel::PushRecent(const std::string& n)
{
    auto it = std::find(m_recent.begin(), m_recent.end(), n);
    if (it != m_recent.end())
    {
        m_recent.erase(it);
    }
    m_recent.push_front(n);
    if ((int)m_recent.size() > kMaxRecent)
    {
        m_recent.pop_back();
    }
}

void ImguiCreationPanel::TriggerCreate(LevelEditorContext* ctx, const Item& it)
{
    if (it.onCreate)
    {
        it.onCreate(ctx);
    }
    PushRecent(it.name);
}

void ImguiCreationPanel::ToggleFavorite(const std::string& name)
{
    if (m_favorites.count(name))
    {
        m_favorites.erase(name);
    }
    else
    {
        m_favorites.insert(name);
    }
}

// --------------------- categories ------------------------
void ImguiCreationPanel::DrawCategories()
{
    std::vector<std::string> cats;
    cats.reserve(16);
    cats.emplace_back("(All)");

    for (auto& it : m_items)
    {
        if (it.category.empty())
        {
            continue;
        }
        if (std::find(cats.begin(), cats.end(), it.category) == cats.end())
        {
            cats.push_back(it.category);
        }
    }
    std::sort(cats.begin() + 1, cats.end());

    if (ImGui::Selectable("(All)", m_activeCategory.empty()))
    {
        m_activeCategory.clear();
    }

    for (auto& c : cats)
    {
        if (c == "(All)")
        {
            continue;
        }
        bool sel = (m_activeCategory == c);
        if (ImGui::Selectable(c.c_str(), sel))
        {
            m_activeCategory = c;
            m_selected = 0;
        }
    }
}
