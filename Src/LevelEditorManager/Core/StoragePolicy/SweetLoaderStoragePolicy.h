#pragma once

#include <string>
#include <optional>
#include <vector>

#include "Utils/SweetLoader/SweetLoader.h"

class SweetLoader;
class LevelEditorContext;
class Level;
struct Entity;

class SweetLoaderStoragePolicy
{
public:
    std::string name() const { return "SweetLoaderStorage"; }

    template<class T> void serialize(const T&) {}
    template<class T> void* clone(const T& t) { return new T(t); }

    bool SaveLevel(const std::string& path, const LevelEditorContext* ctx);
    bool LoadLevel(const std::string& path, LevelEditorContext* ctx);
};
