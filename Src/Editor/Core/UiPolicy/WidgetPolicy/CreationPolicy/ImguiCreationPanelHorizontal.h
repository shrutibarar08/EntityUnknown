#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <deque>
#include <string>

#include "imgui/imgui.h"

class LevelEditorContext;

class ImguiCreationPanelHorizontal
{
public:
    struct Item
    {
        std::string name;
        std::string category;
        std::vector<std::string> tags;
        std::function<void(LevelEditorContext&)> onCreate;
        bool featured = false;
    };

    bool Init(LevelEditorContext* context) { return true; }
    // Public API
    void Register(const Item& it);
    void Register(const std::vector<Item>& items);
    void Clear();

    // Draw (Horizontal layout)
    void DrawCreation(LevelEditorContext* ctx);

    // Optional helpers
    void ToggleFavorite(const std::string& name);
    const std::unordered_set<std::string>& Favorites() const { return m_favorites; }
    const std::deque<std::string>& Recent() const { return m_recent; }

private:
    // Internal state
    std::vector<Item>       m_items;
    std::vector<int>        m_filtered;          // indices into m_items
    std::unordered_set<std::string> m_favorites; // by name
    std::deque<std::string> m_recent;            // MRU by name
    static constexpr int    kMaxRecent = 12;

    // UI state
    char  m_search[128] = { 0 };
    int   m_selected = -1;       // index within filtered list (grid selection)
    int   m_columns = 4;        // grid columns (horizontal)
    bool  m_showFavorites = true;
    bool  m_showFeatured = true;
    bool  m_showRecent = true;

    enum class Tab { All, Favorites, Featured, Recent };
    Tab   m_activeTab = Tab::All;
    std::string m_activeCategory;   // empty => all categories

    // --- utils (defs in .cpp)
    static std::string lower_copy(std::string s);
    static bool contains_ic(const std::string& hay, const std::string& needle);
    static std::vector<std::string> tokenize(const char* q);

    struct ScoredIdx { int idx; int score; };
    int  score_item(const Item& it, const std::vector<std::string>& tokens) const;

    void BuildFiltered();
    void HandleKeyboardGrid();
    void DrawTopBar();
    void DrawCategoryTabs();
    void DrawGrid(LevelEditorContext* ctx);

    void DrawCard(LevelEditorContext* ctx, const Item& it, bool selected);
    void PushRecent(const std::string& n);
    void TriggerCreate(LevelEditorContext* ctx, const Item& it);
    Item* FindByName(const std::string& n);
};
