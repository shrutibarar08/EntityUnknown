#include "EditorStorage.h"
#include "Utils/HelperFunctions.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

#include "Editor/Core/EditorContext.h"

#include <nlohmann/json.hpp>

using StorageHelpers::EnsureProjectAssets;
using StorageHelpers::ReadLevelSceneRelFromManifest;
using StorageHelpers::WriteLevelSceneRelToManifest;
using StorageHelpers::DefaultLevelSceneRel;
using StorageHelpers::MakeAbsoluteFromRel;
using StorageHelpers::ToAssetsRelative;
using StorageHelpers::ManifestPath;
using StorageHelpers::AssetsRoot;

EditorStorage::EditorStorage()
{
    EnsureProjectAssets();
}

bool EditorStorage::Load(LevelEditorContext* ctx)
{
    if (!ctx) return false;
    auto* lm = ctx->GetLevelManager();
    if (!lm) return false;

    const auto rels = ReadManifestLines();
    bool anyLoaded = false;

    for (const auto& rel : rels)
    {
        const std::string abs = MakeAbsoluteFromRel(rel);
        if (LoadLevelFromPath(ctx, abs)) anyLoaded = true;
    }

    return anyLoaded;
}

bool EditorStorage::Save(LevelEditorContext* ctx)
{
    if (!ctx) return false;
    auto* lm = ctx->GetLevelManager();
    if (!lm) return false;

    const auto existingRels = ReadManifestLines();
    const std::vector<std::string> levelNames = lm->GetLevelNames();

    std::vector<std::string> newRels;
    newRels.reserve(levelNames.size());

    for (const auto& name : levelNames)
    {
        // 3) resolve where to save this level
        const std::string relPath = ResolveSaveRelForLevel(name, existingRels);
        const std::string absPath = MakeAbsoluteFromRel(relPath);
        if (!EnsureParentDir(absPath)) continue;

        // 4) persist JSON for this level
        if (SaveLevelToPath(ctx, name, absPath))
            newRels.push_back(relPath);
    }

    // 5) write manifest with updated list
    return WriteManifestLines(newRels);
}

// Replace manifest with exactly one entry (normalized)
bool EditorStorage::SetActiveLevelRel(const std::string& relOrAnyPath)
{
    return WriteManifestLines({ ToAssetsRelative(relOrAnyPath) });
}

// Replace manifest with given entries (normalized, deduped, non-empty)
bool EditorStorage::SetActiveLevelsRel(const std::vector<std::string>& relOrAnyPaths)
{
    std::vector<std::string> rels;
    rels.reserve(relOrAnyPaths.size());
    for (const auto& p : relOrAnyPaths)
    {
        auto r = ToAssetsRelative(p);
        if (!r.empty()) rels.push_back(std::move(r));
    }
    // de-duplicate while preserving order
    std::vector<std::string> unique;
    unique.reserve(rels.size());
    for (auto& r : rels)
        if (std::find(unique.begin(), unique.end(), r) == unique.end())
            unique.push_back(std::move(r));

    return WriteManifestLines(unique);
}

// --------------------------------------------------
// helpers
// --------------------------------------------------

bool EditorStorage::EnsureParentDir(const std::string& absPath) const
{
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(absPath).parent_path(), ec);
    return !ec;
}

std::vector<std::string> EditorStorage::ReadManifestLines() const
{
    std::vector<std::string> out;
    std::ifstream in(ManifestPath(), std::ios::in);
    if (!in) return out;

    std::string line;
    while (std::getline(in, line))
    {
        // allow legacy single-line key format "level_scene=..."
        constexpr const char* kKey = "level_scene=";
        if (line.rfind(kKey, 0) == 0)
        {
            std::string rel = std::string(line.begin() + std::char_traits<char>::length(kKey), line.end());
            rel = ToAssetsRelative(rel);
            if (!rel.empty()) out.push_back(std::move(rel));
            continue;
        }

        // ignore comments/whitespace
        if (line.empty() || line[0] == '#') continue;

        // accept either "Assets/foo.scene" or "foo.scene"
        std::string rel = ToAssetsRelative(line);
        if (!rel.empty()) out.push_back(std::move(rel));
    }
    return out;
}

bool EditorStorage::WriteManifestLines(const std::vector<std::string>& rels) const
{
    std::ofstream out(ManifestPath(), std::ios::out | std::ios::trunc);
    if (!out) return false;

    out << "# application.assets — list of Assets-relative scene files (one per line)\n";
    for (const auto& rel : rels)
        out << rel << "\n";
    return true;
}

bool EditorStorage::LoadLevelFromPath(LevelEditorContext* ctx, const std::string& sceneAbs)
{
    if (!ctx) return false;
    auto* lm = ctx->GetLevelManager();
    if (!lm) return false;

    if (!std::filesystem::exists(sceneAbs))
        return false;

    std::ifstream in(sceneAbs);
    if (!in) return false;

    nlohmann::json j;
    try
    {
        in >> j;
    }
    catch (...) { return false; }

    const std::string sceneName = std::filesystem::path(sceneAbs).stem().string();

    lm->CreateLevel(sceneName);
    if (auto* level = lm->GetLevel(sceneName))
    {
        level->LoadLevelSaveData(j);
        return true;
    }
    return false;
}

bool EditorStorage::SaveLevelToPath(LevelEditorContext* ctx,
    const std::string& levelName,
    const std::string& sceneAbs)
{
    if (!ctx) return false;
    auto* lm = ctx->GetLevelManager();
    if (!lm) return false;

    auto* level = lm->GetLevel(levelName);
    if (!level) return false;

    nlohmann::json j;
    try 
    {
        j = level->GetLevelSaveData();
    }
    catch (...) { return false; }

    std::ofstream out(sceneAbs, std::ios::out | std::ios::trunc);
    if (!out) return false;

    try 
    {
        out << j.dump(2);
    }
    catch (...) { return false; }

    return true;
}

std::string EditorStorage::ResolveSaveRelForLevel(
    const std::string& levelName,
    const std::vector<std::string>& existingRels
) const
{
    auto it = std::find_if(existingRels.begin(), existingRels.end(),
    [&](const std::string& rel)
    {
        return std::filesystem::path(rel).stem().string() == levelName;
    });
    if (it != existingRels.end())
        return *it;

    return levelName + ".scene";
}
