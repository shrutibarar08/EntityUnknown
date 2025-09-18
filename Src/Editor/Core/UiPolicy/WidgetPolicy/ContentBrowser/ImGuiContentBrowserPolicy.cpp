#include "ImGuiContentBrowserPolicy.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <cmath>
#include <string>

#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"

namespace fs = std::filesystem;

static std::string to_lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}
static void kmp_build_lps(const std::string& pat, std::vector<int>& lps)
{
    int len = 0; lps.assign((int)pat.size(), 0);
    for (int i = 1; i < (int)pat.size(); )
        if (pat[i] == pat[len]) lps[i++] = ++len;
        else if (len) len = lps[len - 1];
        else lps[i++] = 0;
}
static bool kmp_contains_icase(const std::string& haystack, const std::string& needle)
{
    if (needle.empty()) return true;
    std::string h = to_lower(haystack), p = to_lower(needle);
    std::vector<int> lps; kmp_build_lps(p, lps);
    int i = 0, j = 0;
    while (i < (int)h.size()) {
        if (h[i] == p[j]) { ++i; ++j; if (j == (int)p.size()) return true; }
        else if (j) j = lps[j - 1];
        else ++i;
    }
    return false;
}
static std::string norm_with_dot(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    if (!s.empty() && s.front() != '.') s.insert(s.begin(), '.');
    return s;
}
static bool should_ignore_ext(const std::vector<std::string>& cfgList, const std::string& extNoDotLower)
{
    const std::string withDot = "." + extNoDotLower;
    for (const std::string& e : cfgList)
        if (norm_with_dot(e) == withDot) return true;
    return false;
}
static std::string to_utf8(const fs::path& p)
{
#if defined(_WIN32)
    auto u8 = p.u8string();
    return std::string(reinterpret_cast<const char*>(u8.c_str()));
#else
    return p.string();
#endif
}
static std::string rel_assets_utf8(const fs::path& abs, const fs::path& root)
{
    std::error_code ec;
    fs::path rel = fs::relative(abs, root, ec);
    std::string s = ec ? to_utf8(abs) : to_utf8(rel);
    for (char& c : s) if (c == '\\') c = '/';
    if (!s.empty() && s.rfind("Assets/", 0) != 0) s = "Assets/" + s;
    return s;
}

ImGuiContentBrowserPolicy::ImGuiContentBrowserPolicy(const Config& cfg)
    : m_cfg(cfg)
{
    SetRoot(cfg.root);
}

bool ImGuiContentBrowserPolicy::Init(LevelEditorContext* /*context*/)
{
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

    RegisterDefaultExtIcons();
    Rescan();
    return true;
}

void ImGuiContentBrowserPolicy::SetRoot(const fs::path& root)
{
    m_root = fs::weakly_canonical(root);
    if (!fs::exists(m_root)) m_root = fs::current_path();
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
    if (m_searchMode) { rebuildItemsForSearch(); return; }
    scanDir(m_current);
}
void ImGuiContentBrowserPolicy::GoUp()
{
    if (m_current == m_root) return;
    enter(m_current.parent_path());
}
void ImGuiContentBrowserPolicy::GoBack()
{
    if (m_histIndex > 0) { --m_histIndex; m_current = m_hist[m_histIndex]; Rescan(); }
}
void ImGuiContentBrowserPolicy::GoForward()
{
    if (m_histIndex + 1 < (int)m_hist.size()) { ++m_histIndex; m_current = m_hist[m_histIndex]; Rescan(); }
}

bool ImGuiContentBrowserPolicy::ParsePayload(const ImGuiPayload* p, PayloadHeader& outHeader, std::string& outPathUtf8)
{
    if (!p || !p->Data || p->DataSize < (int)sizeof(PayloadHeader)) return false;
    if (std::string_view(p->DataType) != std::string_view(kPayloadType)) return false;

    const auto* hdr = reinterpret_cast<const PayloadHeader*>(p->Data);
    if (hdr->magic != 0xA551'7BAD || hdr->version != 1) return false;

    const char* pathz = reinterpret_cast<const char*>(p->Data) + sizeof(PayloadHeader);
    const char* end = reinterpret_cast<const char*>(p->Data) + p->DataSize;
    const char* zero = (const char*)memchr(pathz, 0, (size_t)(end - pathz));
    if (!zero) return false;

    outHeader = *hdr;
    outPathUtf8.assign(pathz);
    return true;
}

void ImGuiContentBrowserPolicy::DrawContentBrowser(LevelEditorContext* /*ctx*/)
{
    toolbar();
    breadcrumbs();

    ImGui::Separator();

    if (m_cfg.gridView) drawGrid();
    else                drawList();
}

void ImGuiContentBrowserPolicy::scanDir(const fs::path& p)
{
    m_items.clear();
    for (fs::directory_iterator it(p), end; it != end; ++it)
    {
        const fs::directory_entry& de = *it;
        if (!m_cfg.showHidden && isHidden(de)) continue;

        Entry e{};
        e.path = de.path();
        e.isDir = de.is_directory();
        e.name = e.path.filename().string();

        if (!e.isDir)
        {
            std::string ext = e.path.extension().string();
            if (!ext.empty() && ext[0] == '.') ext.erase(ext.begin());
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });
            e.ext = std::move(ext);
            if (should_ignore_ext(m_cfg.ignoreExtensions, e.ext)) continue;

            std::error_code ec{};
            auto sz = fs::file_size(e.path, ec);
            e.size = ec ? 0ull : (uint64_t)sz;
        }

        e.icon = iconFor(e);
        m_items.push_back(std::move(e));
    }
}

bool ImGuiContentBrowserPolicy::isHidden(const fs::directory_entry& de)
{
#if defined(_WIN32)
    return de.path().filename().native().starts_with(L".");
#else
    const std::string name = de.path().filename().string();
    return !name.empty() && name[0] == '.';
#endif
}

ImTextureID ImGuiContentBrowserPolicy::iconFor(const Entry& e) const
{
    if (e.isDir) return m_iconFolder ? m_iconFolder : (ImTextureID)0;
    auto it = m_extIcons.find(e.ext);
    if (it != m_extIcons.end() && it->second) return it->second;
    if (m_iconFile) return m_iconFile;
    return m_iconFolder;
}

void ImGuiContentBrowserPolicy::RegisterDefaultExtIcons()
{
    const char* imgExts[] = { "png","jpg","jpeg","bmp","tga","gif","dds","hdr","tif","tiff","webp" };
    const char* meshExts[] = { "obj","fbx","gltf","glb","ply","stl","dae","3ds","blend" };
    const char* shaderExts[] = { "hlsl" };
    const char* textExts[] = { "txt","md","ini","json","yaml","yml","xml","cpp","hpp","h","c","cs","py","lua","js","ts" };

    for (const char* e : imgExts)   SetExtIcon(e, m_iconImage ? m_iconImage : (m_iconFile ? m_iconFile : m_iconFolder));
    for (const char* e : meshExts)  SetExtIcon(e, m_iconMesh ? m_iconMesh : (m_iconFile ? m_iconFile : m_iconFolder));
    for (const char* e : shaderExts)SetExtIcon(e, m_iconShader ? m_iconShader : (m_iconFile ? m_iconFile : m_iconFolder));
    for (const char* e : textExts)  SetExtIcon(e, m_iconFile ? m_iconFile : m_iconFolder);
}

void ImGuiContentBrowserPolicy::toolbar()
{
    if (ImGui::BeginTable("asset_toolbar", 7, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn(); if (ImGui::Button("<")) { GoBack(); }
        ImGui::TableNextColumn(); if (ImGui::Button(">")) { GoForward(); }
        ImGui::TableNextColumn(); if (ImGui::Button("Up")) { GoUp(); }
        ImGui::TableNextColumn(); if (ImGui::Button("Refresh")) { Rescan(); }

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(to_utf8(fs::relative(m_current, m_root)).c_str());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(220.0f);
        static char qbuf[256]{};
        if (m_searchQuery.size() && qbuf[0] == 0) {
            size_t n = std::min(m_searchQuery.size(), sizeof(qbuf) - 1);
            std::memcpy(qbuf, m_searchQuery.data(), n); qbuf[n] = 0;
        }
        bool changed = ImGui::InputTextWithHint("##asset_search_live", "Search...", qbuf, sizeof(qbuf));
        if (changed)
        {
            m_searchQuery = qbuf;
            m_searchMode = !m_searchQuery.empty();
            if (m_searchMode) rebuildItemsForSearch();
            else              scanDir(m_current);
        }

        ImGui::TableNextColumn();
        if (ImGui::Button("Clear") && (!m_searchQuery.empty() || qbuf[0] != 0))
        {
            m_searchQuery.clear();
            qbuf[0] = 0;
            m_searchMode = false;
            scanDir(m_current);
        }

        ImGui::EndTable();
    }

    if (m_searchMode) { ImGui::SameLine(); ImGui::TextDisabled("Results: %zu", m_items.size()); }
}

void ImGuiContentBrowserPolicy::breadcrumbs()
{
    if (ImGui::Button("Assets")) enter(m_root);
    ImGui::SameLine();

    fs::path rel;
    std::vector<std::string> parts;
    for (const auto& comp : m_current.lexically_relative(m_root))
        parts.push_back(to_utf8(comp));

    for (size_t i = 0; i < parts.size(); ++i)
    {
        rel /= parts[i];
        ImGui::SameLine();
        if (ImGui::Button(parts[i].c_str())) enter(m_root / rel);
        if (i + 1 < parts.size()) { ImGui::SameLine(); ImGui::TextUnformatted("/"); }
    }
}

void ImGuiContentBrowserPolicy::drawGrid()
{
    const float cell = m_cfg.tileSize + m_cfg.padding;
    const float avail = ImGui::GetContentRegionAvail().x;
    int columns = std::clamp((int)std::floor(avail / cell), 1, 32);

    fs::path pending_nav;

    if (ImGui::BeginTable("asset_grid", columns, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX))
    {
        int idx = 0;
        for (auto& e : m_items)
        {
            ImGui::TableNextColumn();
            ImGui::PushID(idx++);

            ImVec2 sz{ m_cfg.tileSize, m_cfg.tileSize };
            bool activated = false;

            if (e.icon) activated = ImGui::ImageButton("icon", e.icon, sz);
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1));
                activated = ImGui::Button(e.isDir ? "[DIR]" : "[FILE]", sz);
                ImGui::PopStyleColor();
            }

            if (activated && e.isDir) pending_nav = e.path;

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                m_dragBuf.clear();
                PayloadHeader hdr{};
                hdr.kind = static_cast<uint16_t>(e.isDir ? Kind::Directory : Kind::File);

                const std::string relStr = rel_assets_utf8(e.path, m_root);

                m_dragBuf.resize(sizeof(PayloadHeader) + relStr.size() + 1);
                memcpy(m_dragBuf.data(), &hdr, sizeof(PayloadHeader));
                memcpy(m_dragBuf.data() + sizeof(PayloadHeader), relStr.c_str(), relStr.size() + 1);

                ImGui::SetDragDropPayload(kPayloadType, m_dragBuf.data(), (int)m_dragBuf.size());
                ImGui::TextUnformatted(relStr.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_cfg.tileSize);
            if (m_cfg.showExtensions || e.isDir) ImGui::TextUnformatted(e.name.c_str());
            else {
                const size_t dot = e.name.rfind('.');
                ImGui::TextUnformatted(dot == std::string::npos ? e.name.c_str() : e.name.substr(0, dot).c_str());
            }
            ImGui::PopTextWrapPos();

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (!pending_nav.empty()) enter(pending_nav);
}

void ImGuiContentBrowserPolicy::drawList()
{
    fs::path pending_nav;

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
                if (e.isDir) pending_nav = e.path;

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                m_dragBuf.clear();
                PayloadHeader hdr{};
                hdr.kind = static_cast<uint16_t>(e.isDir ? Kind::Directory : Kind::File);

                const std::string relStr = rel_assets_utf8(e.path, m_root);

                m_dragBuf.resize(sizeof(PayloadHeader) + relStr.size() + 1);
                memcpy(m_dragBuf.data(), &hdr, sizeof(PayloadHeader));
                memcpy(m_dragBuf.data() + sizeof(PayloadHeader), relStr.c_str(), relStr.size() + 1);

                ImGui::SetDragDropPayload(kPayloadType, m_dragBuf.data(), (int)m_dragBuf.size());
                ImGui::TextUnformatted(relStr.c_str());
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

    if (!pending_nav.empty()) enter(pending_nav);
}

void ImGuiContentBrowserPolicy::enter(const fs::path& child)
{
    std::error_code ec;
    fs::path canon = fs::weakly_canonical(child, ec);
    if (ec || !fs::exists(canon) || !fs::is_directory(canon)) return;

    fs::path rel = fs::relative(canon, m_root, ec);
    if (ec || rel.string().starts_with("..")) return;

    m_current = canon;

    if (m_histIndex + 1 < (int)m_hist.size())
        m_hist.erase(m_hist.begin() + m_histIndex + 1, m_hist.end());
    m_hist.push_back(m_current);
    m_histIndex = (int)m_hist.size() - 1;

    if (m_searchMode) rebuildItemsForSearch();
    else              scanDir(m_current);
}

void ImGuiContentBrowserPolicy::rebuildItemsForSearch()
{
    m_items.clear();
    m_searchMode = !m_searchQuery.empty();
    if (!m_searchMode) { scanDir(m_current); return; }

    std::error_code ec;
    const std::string pat = m_searchQuery;

    for (fs::recursive_directory_iterator it(m_root, fs::directory_options::skip_permission_denied, ec), end;
        it != end; it.increment(ec))
    {
        const fs::path& p = it->path();

        if (!m_cfg.showHidden) {
#if defined(_WIN32)
            if (p.filename().native().starts_with(L".")) continue;
#else
            if (!p.filename().empty() && p.filename().c_str()[0] == '.') continue;
#endif
        }

        bool isDir = it->is_directory(ec);
        if (!isDir) {
            std::string ext = p.extension().string();
            if (!ext.empty() && ext[0] == '.') ext.erase(ext.begin());
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });
            if (should_ignore_ext(m_cfg.ignoreExtensions, ext)) continue;
        }

        const std::string name = to_utf8(p.filename());
        const std::string rel = rel_assets_utf8(p, m_root);

        if (kmp_contains_icase(name, pat) || kmp_contains_icase(rel, pat))
        {
            Entry e{};
            e.path = p;
            e.isDir = isDir;
            e.name = p.filename().string();
            if (!e.isDir) {
                std::string ext = p.extension().string();
                if (!ext.empty() && ext[0] == '.') ext.erase(ext.begin());
                std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c) { return (char)std::tolower(c); });
                e.ext = std::move(ext);
                std::error_code fec{};
                auto sz = fs::file_size(p, fec);
                e.size = fec ? 0ull : (uint64_t)sz;
            }
            e.icon = iconFor(e);
            m_items.push_back(std::move(e));
        }
    }
}
