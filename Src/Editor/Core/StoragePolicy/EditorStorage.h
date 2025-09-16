#pragma once
#include <string>
#include <vector>

class LevelEditorContext;

class EditorStorage
{
public:
    EditorStorage();

    bool Load(LevelEditorContext* ctx);
    bool Save(LevelEditorContext* ctx);

    bool SetActiveLevelRel(const std::string& relOrAnyPath);
    bool SetActiveLevelsRel(const std::vector<std::string>& relOrAnyPaths);

private:
    bool EnsureParentDir(const std::string& absPath) const;
    std::vector<std::string> ReadManifestLines() const;
    bool WriteManifestLines(const std::vector<std::string>& rels) const;
    bool LoadLevelFromPath(LevelEditorContext* ctx, const std::string& sceneAbs);
    bool SaveLevelToPath(LevelEditorContext* ctx,
        const std::string& levelName,
        const std::string& sceneAbs);
    std::string ResolveSaveRelForLevel(const std::string& levelName,
        const std::vector<std::string>& existingRels) const;
};
