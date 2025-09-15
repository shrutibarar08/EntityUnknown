#include "ImGuiContentBrowserPolicy.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <cmath>
#include <string>

#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"

namespace fs = std::filesystem;

// ---------- tiny helpers ----------
static std::string to_utf8(const fs::path& p) {
#if defined(_WIN32)
    // C++20: path::u8string() is fine; returns UTF-8
    auto u8 = p.u8string();
    return std::string(reinterpret_cast<const char*>(u8.c_str()));
#else
    return p.string();
#endif
}
static std::string lower_no_dot(std::string s) 
{
    if (!s.empty() && s[0] == '.') s.erase(s.begin());
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}
// -----------------------------------

ImGuiContentBrowserPolicy::ImGuiContentBrowserPolicy(const Config& cfg)
    : m_cfg(cfg)
{
    SetRoot(cfg.root);
}

bool ImGuiContentBrowserPolicy::Init(LevelEditorContext* /*context*/)
{
    // Load core icons (if paths provided)
    auto loadSRV = [](const char* path) -> ImTextureID {
        if (!path || !*path) return (ImTextureID)0;
        auto tex = TextureLoader::GetTexture(path);
        return tex.IsInitialized() ? (ImTextureID)tex.ShaderResourceView : (ImTextureID)0;
        };

    m_iconFolder = loadSRV(m_cfg.iconFolderPath);
    m_iconFile = loadSRV(m_cfg.iconFilePath);
    m_iconImage = loadSRV(m_cfg.iconImagePath);
    m_iconMesh = loadSRV(m_cfg.iconMeshPath);
    m_iconShader = loadSRV(m_cfg.iconShaderPath);

    // Register default extension mappings
    RegisterDefaultExtIcons();

    // Initial scan
    Rescan();
    return true;
}


void ImGuiContentBrowserPolicy::SetRoot(const fs::path& root)
{
    m_root = fs::weakly_canonical(root);
    if (!fs::exists(m_root)) {
        // don’t create silently; just fallback to current_path
        m_root = fs::current_path();
    }
    m_current = m_root;
    m_hist.clear();
    m_hist.push_back(m_current);
    m_histIndex = 0;
    Rescan();
}

void ImGuiContentBrowserPolicy::SetIcons(ImTextureID folder, ImTextureID file)
{
    m_iconFolder = folder;
    m_iconFile = file;
}

void ImGuiContentBrowserPolicy::SetExtIcon(std::string extNoDotLower, ImTextureID icon)
{
    std::transform(extNoDotLower.begin(), extNoDotLower.end(), extNoDotLower.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    if (!extNoDotLower.empty() && extNoDotLower.front() == '.')
        extNoDotLower.erase(extNoDotLower.begin());

    m_extIcons[extNoDotLower] = icon;
}

void ImGuiContentBrowserPolicy::Rescan()
{
    scanDir(m_current);
}

void ImGuiContentBrowserPolicy::GoUp()
{
    if (m_current == m_root) return;
    enter(m_current.parent_path());
}

void ImGuiContentBrowserPolicy::GoBack()
{
    if (m_histIndex > 0) {
        m_histIndex--;
        m_current = m_hist[m_histIndex];
        Rescan();
    }
}

void ImGuiContentBrowserPolicy::GoForward()
{
    if (m_histIndex + 1 < (int)m_hist.size()) {
        m_histIndex++;
        m_current = m_hist[m_histIndex];
        Rescan();
    }
}

bool ImGuiContentBrowserPolicy::ParsePayload(const ImGuiPayload* p, PayloadHeader& outHeader, std::string& outPathUtf8)
{
    if (!p || !p->Data || p->DataSize < (int)sizeof(PayloadHeader)) return false;
    if (std::string_view(p->DataType) != std::string_view(kPayloadType)) return false;

    const auto* hdr = reinterpret_cast<const PayloadHeader*>(p->Data);
    if (hdr->magic != 0xA551'7BAD || hdr->version != 1) return false;

    const char* pathz = reinterpret_cast<const char*>(p->Data) + sizeof(PayloadHeader);
    const char* end = reinterpret_cast<const char*>(p->Data) + p->DataSize;
    // ensure zero-terminated within range
    const char* zero = (const char*)memchr(pathz, 0, (size_t)(end - pathz));
    if (!zero) return false;

    outHeader = *hdr;
    outPathUtf8.assign(pathz);
    return true;
}

void ImGuiContentBrowserPolicy::DrawContentBrowser(LevelEditorContext* /*ctx*/)
{
    // Toolbar + breadcrumbs
    toolbar();
    breadcrumbs();

    ImGui::Separator();

    if (m_cfg.gridView) drawGrid();
    else                drawList();
}

// ---------------- internals ----------------

void ImGuiContentBrowserPolicy::scanDir(const std::filesystem::path& p)
{
    m_items.clear();
    for (auto& de : std::filesystem::directory_iterator(p))
    {
        if (!m_cfg.showHidden && isHidden(de)) continue;

        Entry e;
        e.path = de.path();
        e.isDir = de.is_directory();

        e.name = e.path.filename().string();

        // normalize extension: no dot, lowercased
        if (!e.isDir)
        {
            auto ext = e.path.extension().string(); // includes dot
            if (!ext.empty() && ext[0] == '.') ext.erase(ext.begin());
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });
            e.ext = std::move(ext);
        }

        e.icon = iconFor(e);
        m_items.push_back(std::move(e));
    }
}

bool ImGuiContentBrowserPolicy::isHidden(const fs::directory_entry& de)
{
#if defined(_WIN32)
    // Cheap heuristic: leading dot, since querying FILE_ATTRIBUTE_HIDDEN needs Win32
    return de.path().filename().native().starts_with(L".");
#else
    return de.path().filename().c_str()[0] == '.';
#endif
}

ImTextureID ImGuiContentBrowserPolicy::iconFor(const Entry& e) const
{
    if (e.isDir) return m_iconFolder ? m_iconFolder : (ImTextureID)0;

    // ext already normalized at scan time; fall back to file, then folder
    auto it = m_extIcons.find(e.ext);
    if (it != m_extIcons.end() && it->second) return it->second;
    if (m_iconFile) return m_iconFile;
    return m_iconFolder;
}

void ImGuiContentBrowserPolicy::RegisterDefaultExtIcons()
{
    // Images
    const char* imgExts[] = { "png","jpg","jpeg","bmp","tga","gif","dds","hdr","tif","tiff","webp" };
    for (auto* e : imgExts) SetExtIcon(e, m_iconImage ? m_iconImage : (m_iconFile ? m_iconFile : m_iconFolder));

    // Meshes / 3D
    const char* meshExts[] = { "obj","fbx","gltf","glb","ply","stl","dae","3ds","blend" };
    for (auto* e : meshExts) SetExtIcon(e, m_iconMesh ? m_iconMesh : (m_iconFile ? m_iconFile : m_iconFolder));

    // Shaders
    const char* shaderExts[] = { "hlsl" /* add more if needed: "fx","fxh","usf","ush" */ };
    for (auto* e : shaderExts) SetExtIcon(e, m_iconShader ? m_iconShader : (m_iconFile ? m_iconFile : m_iconFolder));

    // Common text/code (optional bonus)
    const char* textExts[] = { "txt","md","ini","json","yaml","yml","xml","cpp","hpp","h","c","cs","py","lua","js","ts" };
    for (auto* e : textExts) SetExtIcon(e, m_iconFile ? m_iconFile : m_iconFolder);
}

void ImGuiContentBrowserPolicy::toolbar()
{
    if (ImGui::BeginTable("asset_toolbar", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Button("<")) GoBack();
        ImGui::TableNextColumn();
        if (ImGui::Button(">")) GoForward();
        ImGui::TableNextColumn();
        if (ImGui::Button("Up")) GoUp();
        ImGui::TableNextColumn();
        if (ImGui::Button("Refresh")) Rescan();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(to_utf8(std::filesystem::relative(m_current, m_root)).c_str());
        ImGui::EndTable();
    }
}

void ImGuiContentBrowserPolicy::breadcrumbs()
{
    // Root crumb
    if (ImGui::Button("Assets")) {
        enter(m_root);
    }
    ImGui::SameLine();

    // Each component
    fs::path rel;
    std::vector<std::string> parts;
    for (const auto& comp : m_current.lexically_relative(m_root)) {
        parts.push_back(to_utf8(comp));
    }
    for (size_t i = 0; i < parts.size(); ++i) {
        rel /= parts[i];
        ImGui::SameLine();
        if (ImGui::Button(parts[i].c_str())) {
            enter(m_root / rel);
        }
        if (i + 1 < parts.size()) { ImGui::SameLine(); ImGui::TextUnformatted("/"); }
    }
}

void ImGuiContentBrowserPolicy::drawGrid()
{
    const float cell = m_cfg.tileSize + m_cfg.padding;
    const float avail = ImGui::GetContentRegionAvail().x;
    int columns = (int)std::floor(avail / cell);
    columns = std::clamp(columns, 1, 32);

    std::filesystem::path pending_nav; // <— defer navigation

    if (ImGui::BeginTable("asset_grid", columns,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX))
    {
        int idx = 0;
        for (auto& e : m_items)
        {
            ImGui::TableNextColumn();
            ImGui::PushID(idx++);

            ImVec2 sz{ m_cfg.tileSize, m_cfg.tileSize };
            bool activated = false;

            if (e.icon)
                activated = ImGui::ImageButton("icon", e.icon, sz); // PushID makes "icon" unique
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1));
                activated = ImGui::Button(e.isDir ? "[DIR]" : "[FILE]", sz);
                ImGui::PopStyleColor();
            }

            if (activated && e.isDir)
                pending_nav = e.path; // <— just remember

            // Drag source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                m_dragBuf.clear();
                PayloadHeader hdr{};
                hdr.kind = static_cast<uint16_t>(e.isDir ? Kind::Directory : Kind::File);

                const std::string pathUtf8 = to_utf8(e.path);
                m_dragBuf.resize(sizeof(PayloadHeader) + pathUtf8.size() + 1);
                memcpy(m_dragBuf.data(), &hdr, sizeof(PayloadHeader));
                memcpy(m_dragBuf.data() + sizeof(PayloadHeader), pathUtf8.c_str(), pathUtf8.size() + 1);

                ImGui::SetDragDropPayload(kPayloadType, m_dragBuf.data(), (int)m_dragBuf.size());
                ImGui::TextUnformatted(pathUtf8.c_str());
                ImGui::EndDragDropSource();
            }

            // Name under tile
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_cfg.tileSize);
            if (m_cfg.showExtensions || e.isDir)
                ImGui::TextUnformatted(e.name.c_str());
            else
            {
                const size_t dot = e.name.rfind('.');
                ImGui::TextUnformatted(dot == std::string::npos ? e.name.c_str()
                    : e.name.substr(0, dot).c_str());
            }
            ImGui::PopTextWrapPos();

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    // Apply navigation AFTER UI finished using m_items
    if (!pending_nav.empty())
        enter(pending_nav);
}

void ImGuiContentBrowserPolicy::drawList()
{
    std::filesystem::path pending_nav; // <— defer navigation

    if (ImGui::BeginTable("asset_list", 3, m_cfg.listFlags))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100.f);
        ImGui::TableHeadersRow();

        int idx = 0;
        for (auto& e : m_items)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(idx++);

            if (e.icon) { ImGui::Image(e.icon, ImVec2(16, 16)); ImGui::SameLine(); }
            if (ImGui::Selectable(e.name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
                if (e.isDir) pending_nav = e.path; // <— defer

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                m_dragBuf.clear();
                PayloadHeader hdr{};
                hdr.kind = static_cast<uint16_t>(e.isDir ? Kind::Directory : Kind::File);

                const std::string pathUtf8 = to_utf8(e.path);
                m_dragBuf.resize(sizeof(PayloadHeader) + pathUtf8.size() + 1);
                memcpy(m_dragBuf.data(), &hdr, sizeof(PayloadHeader));
                memcpy(m_dragBuf.data() + sizeof(PayloadHeader), pathUtf8.c_str(), pathUtf8.size() + 1);

                ImGui::SetDragDropPayload(kPayloadType, m_dragBuf.data(), (int)m_dragBuf.size());
                ImGui::TextUnformatted(pathUtf8.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(e.isDir ? "Folder" : (e.ext.empty() ? "File" : e.ext.c_str()));

            ImGui::TableNextColumn();
            if (!e.isDir) ImGui::Text("%llu", (unsigned long long)e.size);

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (!pending_nav.empty())
        enter(pending_nav);
}

void ImGuiContentBrowserPolicy::enter(const fs::path& child)
{
    fs::path canon;
    std::error_code ec;
    canon = fs::weakly_canonical(child, ec);
    if (ec || !fs::exists(canon) || !fs::is_directory(canon)) return;

    // prevent leaving root
    auto rel = fs::relative(canon, m_root, ec);
    if (ec || rel.string().starts_with("..")) return;

    m_current = canon;

    // push into history (truncate forward)
    if (m_histIndex + 1 < (int)m_hist.size())
        m_hist.erase(m_hist.begin() + m_histIndex + 1, m_hist.end());
    m_hist.push_back(m_current);
    m_histIndex = (int)m_hist.size() - 1;

    Rescan();
}
