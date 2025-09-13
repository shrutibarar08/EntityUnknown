#include "SweetLoaderStoragePolicy.h"

#include <utility>
#include <string>
#include <unordered_map>

#include "Utils/SweetLoader/SweetLoader.h"
#include "LevelEditorManager/Core/EditorContext.h"
#include "LevelEditorManager/Core/LevelManager/LevelModel.h"

bool SweetLoaderStoragePolicy::SaveLevel(const std::string& path, const LevelEditorContext* ctx)
{
    return true;
}

bool SweetLoaderStoragePolicy::LoadLevel(const std::string& path, LevelEditorContext* ctx)
{
    return true;
}
