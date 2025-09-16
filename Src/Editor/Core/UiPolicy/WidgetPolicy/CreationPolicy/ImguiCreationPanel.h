#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <deque>

#include "imgui/imgui.h" // adjust include path if needed

class LevelEditorContext;

class ImguiCreationPanel
{
public:
    struct Item
    {
        std::string name;
        std::string category;
        std::vector<std::string> tags;
        std::function<void(LevelEditorContext*)> onCreate;
        bool featured = false;
    };

    bool Init(LevelEditorContext* context);

    void Register(const Item& it);
    void Register(const std::vector<Item>& items);
    void Clear();

    void DrawCreation(LevelEditorContext* ctx);

    void ToggleFavorite(const std::string& name);
    const std::unordered_set<std::string>& Favorites() const { return m_favorites; }
    const std::deque<std::string>& Recent() const { return m_recent; }

private:
    enum class Section { Results, Favorites, Featured, Recent };

    std::vector<Item>       m_items;
    std::vector<int>        m_filtered;
    std::unordered_set<std::string> m_favorites;
    std::deque<std::string> m_recent;
    static constexpr int    kMaxRecent = 10;

    char  m_search[128] = { 0 };
    int   m_selected = -1;
    bool  m_showFeatured = true;
    bool  m_showCategories = true;
    bool  m_showFavorites = true;
    std::string m_activeCategory;

    static std::string LowerCopy(std::string s);
    static bool ContainsIC(const std::string& hay, const std::string& needle);
    static std::vector<std::string> Tokenize(const char* q);

    struct ScoredIdx { int idx; int score; };
    int  ScoreItem(const Item& it, const std::vector<std::string>& tokens) const;

    void BuildFiltered();
    void HandleKeyboard();

    void DrawList(LevelEditorContext* ctx, Section sec);
    void DrawResults(LevelEditorContext* ctx);
    void DrawFeatured(LevelEditorContext* ctx);
    void DrawFavorites(LevelEditorContext* ctx);
    void DrawRecent(LevelEditorContext* ctx);
    void DrawCard(LevelEditorContext* ctx, const Item& it, Section sec);

    Item* FindByName(const std::string& n);
    void  PushRecent(const std::string& n);
    void  TriggerCreate(LevelEditorContext* ctx, const Item& it);
    void  DrawCategories();
};
