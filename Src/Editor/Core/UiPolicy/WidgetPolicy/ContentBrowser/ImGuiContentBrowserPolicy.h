#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <cstdint>
#include <imgui/imgui.h>

class LevelEditorContext;
struct TEXTURE_RESOURCE;

class ImGuiContentBrowserPolicy
{
public:
    struct Config
    {
        std::filesystem::path root = "Assets";
        float  tileSize = 96.0f;
        float  padding = 8.0f;
        bool   gridView = true;
        bool   showHidden = false; // “.” files
        bool   showExtensions = true;
        ImGuiTableFlags listFlags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;

        // Optional icon paths (Assets-relative or absolute). If empty, no custom icon is loaded.
        const char* iconFolderPath  = "Assets/UI/folder_icon.png";
        const char* iconFilePath    = "Assets/UI/file.png";
        const char* iconImagePath   = "Assets/UI/image.png";
        const char* iconMeshPath    = "Assets/UI/mesh.png";
        const char* iconShaderPath  = "Assets/UI/shader.png";
    };

    static constexpr const char* kPayloadType = "LE_ASSET_V1";
    enum class Kind : uint16_t { File = 0, Directory = 1 };
#pragma pack(push,1)
    struct PayloadHeader
    {
        uint32_t magic{ 0xA551'7BAD };
        uint16_t version{ 1 };
        uint16_t kind{ 0 };
    };
#pragma pack(pop)

public:
    explicit ImGuiContentBrowserPolicy(const Config& cfg = {});
    ~ImGuiContentBrowserPolicy() = default;

    bool Init(LevelEditorContext* context);

    // Setup
    void SetRoot(const std::filesystem::path& root);
    void SetIcons(ImTextureID folder, ImTextureID file);
    void SetExtIcon(std::string extNoDotLower, ImTextureID icon);

    void DrawContentBrowser(LevelEditorContext* ctx);

    // Helpers
    const std::filesystem::path& Current() const { return m_current; }
    void Rescan();
    void GoUp();
    void GoBack();
    void GoForward();

    static bool ParsePayload(const ImGuiPayload* payload, PayloadHeader& outHeader, std::string& outPathUtf8);

private:
    struct Entry
    {
        std::filesystem::path path;
        std::string name;
        std::string ext;
        bool        isDir{ false };
        uint64_t    size{ 0 };
        ImTextureID icon{ 0 };
    };

    // internal
    void scanDir(const std::filesystem::path& p);
    void toolbar();
    void breadcrumbs();
    void drawGrid();
    void drawList();
    void enter(const std::filesystem::path& child);
    static bool isHidden(const std::filesystem::directory_entry& de);
    ImTextureID iconFor(const Entry&) const;

    void RegisterDefaultExtIcons();

private:
    Config m_cfg;

    std::filesystem::path m_root;
    std::filesystem::path m_current;

    std::vector<std::filesystem::path> m_hist;
    int m_histIndex = -1;

    std::vector<Entry> m_items;

    // icons
    ImTextureID m_iconFolder{ 0 };
    ImTextureID m_iconFile{ 0 };
    ImTextureID m_iconImage{ 0 };
    ImTextureID m_iconMesh{ 0 };
    ImTextureID m_iconShader{ 0 };
    std::unordered_map<std::string, ImTextureID> m_extIcons;

    std::vector<char> m_dragBuf;
};
