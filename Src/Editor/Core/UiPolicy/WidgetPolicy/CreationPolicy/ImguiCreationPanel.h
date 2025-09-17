#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <deque>

#include "imgui/imgui.h"

class LevelEditorContext;
struct ID3D11ShaderResourceView;

class ImguiCreationPanel
{
public:
    struct Item
    {
        std::string name;
        std::string category;
        std::vector<std::string> tags;
        std::function<void(LevelEditorContext*)> onCreate;
        bool featured{ false };
    };

    struct Config
    {
        std::unordered_map<std::string, std::string> iconPaths =
        {

        };
    };

    bool Init(LevelEditorContext* context);
    void SetConfig(const Config& cfg) { m_config = cfg; }

    void Register(const Item& it);
    void Register(const std::vector<Item>& items);
    void Clear();

    void DrawCreation(LevelEditorContext* ctx);

    void ToggleFavorite(const std::string& name);
    const std::unordered_set<std::string>& Favorites() const { return m_favorites; }
    const std::deque<std::string>& Recent() const { return m_recent; }

private:
    enum class Section { Results, Favorites, Featured, Recent };

    static std::string LowerCopy(std::string s);
    static bool ContainsIC(const std::string& hay, const std::string& needle);
    static std::vector<std::string> Tokenize(const char* q);

    struct ScoredIdx { int idx; int score; };
    int  ScoreItem(const Item& it, const std::vector<std::string>& tokens) const;

    void BuildFiltered ();
    void HandleKeyboard();

    void DrawList       (LevelEditorContext* ctx, Section sec);
    void DrawResults    (LevelEditorContext* ctx);
    void DrawFeatured   (LevelEditorContext* ctx);
    void DrawFavorites  (LevelEditorContext* ctx);
    void DrawRecent     (LevelEditorContext* ctx);
    void DrawCard       (LevelEditorContext* ctx, const Item& it, Section sec);

    Item* FindByName(const std::string& n);
    void  PushRecent(const std::string& n);
    void  TriggerCreate(LevelEditorContext* ctx, const Item& it);
    void  DrawCategories();
    void  DrawToolbarLeft(ImguiCreationPanel* self);

    void LoadIconsIfNeeded();

private:
    std::vector<Item>               m_items;
    std::vector<int>                m_filtered;
    std::unordered_set<std::string> m_favorites;
    std::deque<std::string>         m_recent;
    char                            m_search[128]{ 0 };
    int                             m_selected{ -1 };
    bool                            m_showFeatured{ true };
    bool                            m_showCategories{ true };
    bool                            m_showFavorites{ true };
    std::string                     m_activeCategory;

    struct Icons
    {
        ID3D11ShaderResourceView* light { nullptr };
        ID3D11ShaderResourceView* mesh  { nullptr };
        ID3D11ShaderResourceView* sprite{ nullptr };
        ID3D11ShaderResourceView* favOn { nullptr };
        ID3D11ShaderResourceView* favOff{ nullptr };
        ID3D11ShaderResourceView* create{ nullptr };
        ID3D11ShaderResourceView* clear { nullptr };
        bool ready{ false };
    } m_icons;

    Config m_config{};
    static constexpr int kMaxRecent = 10;
};
