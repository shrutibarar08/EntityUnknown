// AssetBrowserPanel.h (test-only)
#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "Imgui/imgui.h"

struct AssetEntry { std::string name, path; bool isDir = false; };

class AssetBrowserPanel {
public:
    void SetRoot(std::string root) {
        m_root = std::move(root);
        m_current = m_root;
        Rescan();
    }
    void SetIcons(ImTextureID folder, ImTextureID file) { m_iconFolder = folder; m_iconFile = file; }
    void SetTile(float size, float pad) { m_tileSize = size; m_padding = pad; }

    void Draw() {
        if (!ImGui::Begin("Assets")) { ImGui::End(); return; }

        // Toolbar
        if (ImGui::Button("Up")) { GoUp(); }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) { Rescan(); }
        ImGui::SameLine();
        ImGui::TextUnformatted(m_current.c_str());
        ImGui::Separator();

        // Grid
        const float cell = m_tileSize + m_padding;
        const float avail = ImGui::GetContentRegionAvail().x;
        int columns = static_cast<int>(std::floor(avail / cell));
        columns = std::clamp(columns, 1, 32);

        // NEW: defer folder enter
        std::string pendingEnter;

        if (ImGui::BeginTable("asset_grid", columns, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX)) {
            int idx = 0;
            for (const auto& e : m_items) { // iterate by const ref; don't mutate m_items here
                ImGui::TableNextColumn();
                ImGui::PushID(idx++);

                ImTextureID icon = e.isDir ? m_iconFolder : m_iconFile;
                ImVec2 sz{ m_tileSize, m_tileSize };

                bool activated = false;
#if IMGUI_VERSION_NUM >= 19000
                if (icon) activated = ImGui::ImageButton("##icon", icon, sz);
#else
                if (icon) activated = ImGui::ImageButton(icon, sz);
#endif
                if (!icon) {
                    activated = ImGui::Button(e.isDir ? "[DIR]" : "[FILE]", sz) || activated;
                }

                // DEFER: don't call Rescan() here
                if (activated && e.isDir) {
                    pendingEnter = e.path; // will navigate after we finish drawing
                }

                // Drag source for files
                if (!e.isDir && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("ASSET_PATH", e.path.c_str(), e.path.size() + 1);
                    ImGui::TextUnformatted(e.name.c_str());
                    ImGui::EndDragDropSource();
                }

                // Name under the tile
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_tileSize);
                ImGui::TextUnformatted(e.name.c_str());
                ImGui::PopTextWrapPos();

                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        // SAFE PLACE to mutate m_items
        if (!pendingEnter.empty()) {
            m_current = std::move(pendingEnter);
            Rescan();
        }

        ImGui::End();
    }

private:
    void GoUp() {
        namespace fs = std::filesystem;
        fs::path cur = fs::path(m_current);
        fs::path root = fs::path(m_root);
        if (cur == root) return;
        cur = cur.parent_path();
        if (cur.empty()) return;
        m_current = cur.generic_string();
        Rescan();
    }

    void Rescan() {
        namespace fs = std::filesystem;
        m_items.clear();
        std::error_code ec;
        if (!fs::exists(m_current, ec)) return;

        // List dirs first, then files
        for (auto it = fs::directory_iterator(m_current, ec); it != fs::directory_iterator(); it.increment(ec)) {
            if (ec) continue;
            AssetEntry e;
            e.isDir = it->is_directory(ec);
            e.path = it->path().generic_string();
            e.name = it->path().filename().generic_string();
            m_items.push_back(std::move(e));
        }
        std::stable_sort(m_items.begin(), m_items.end(),
            [](const AssetEntry& a, const AssetEntry& b) {
                if (a.isDir != b.isDir) return a.isDir > b.isDir; // dirs first
                return a.name < b.name;
            });
    }

private:
    std::string m_root = "assets";
    std::string m_current = "assets";
    std::vector<AssetEntry> m_items;

    ImTextureID m_iconFolder;
    ImTextureID m_iconFile;
    float m_tileSize = 72.0f;
    float m_padding = 8.0f;
};
