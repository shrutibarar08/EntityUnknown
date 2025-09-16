#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <filesystem>
#include <fstream>


static std::string WideToUTF8(const std::wstring& w)
{
    if (w.empty()) return {};
    int size = ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string out(size, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), out.data(), size, nullptr, nullptr);
    return out;
}


template<class...Ts> struct type_list{};
constexpr uint64_t ct_hash(const char* str)
{
    uint64_t hash = 1469598103934665603ull;
    while (*str)
    {
        hash ^= static_cast<unsigned char>(*str++);
        hash *= 1099511628211ull;
    }
    return hash;
}

namespace StorageHelpers
{
    inline std::string AssetsRoot()
    {
        return (std::filesystem::current_path() / "Assets").string();
    }

    inline std::string ManifestPath()
    {
        return (std::filesystem::path(AssetsRoot()) / "application.assets").string();
    }

    inline std::string DefaultLevelSceneRel()
    {
        return "DefaultLevel.scene";
    }

    inline std::string MakeAbsoluteFromRel(const std::string& rel)
    {
        return (std::filesystem::path(AssetsRoot()) / std::filesystem::path(rel)).string();
    }

    // Convert to Assets-relative path (no "Assets/" prefix).
    inline std::string ToAssetsRelative(const std::string& in)
    {
        if (in.empty()) return {};

        std::filesystem::path p(in);
        std::filesystem::path assets(AssetsRoot());

        std::error_code ec;
        auto rel = std::filesystem::relative(p, assets, ec);
        if (!ec && !rel.empty() && rel.native()[0] != '.')
            return rel.generic_string();

        if (!p.is_absolute())
        {
            if (!p.empty() && p.begin()->string() == "Assets")
            {
                std::filesystem::path stripped;
                for (auto it = ++p.begin(); it != p.end(); ++it) stripped /= *it;
                return stripped.generic_string();
            }
            return p.generic_string();
        }

        return p.filename().generic_string();
    }

    inline void EnsureProjectAssets()
    {
        std::filesystem::create_directories(AssetsRoot());
        const std::string manifest = ManifestPath();
        if (!std::filesystem::exists(manifest))
        {
            std::ofstream out(manifest, std::ios::out | std::ios::trunc);
            if (out) out << "level_scene=" << DefaultLevelSceneRel() << "\n";
        }
    }

    inline std::string ReadLevelSceneRelFromManifest()
    {
        std::ifstream in(ManifestPath(), std::ios::in);
        if (!in) return {};
        std::string line;
        while (std::getline(in, line))
        {
            constexpr const char* kKey = "level_scene=";
            if (line.rfind(kKey, 0) == 0)
                return std::string(line.begin() + std::char_traits<char>::length(kKey), line.end());
        }
        return {};
    }

    inline bool WriteLevelSceneRelToManifest(const std::string& rel)
    {
        std::ofstream out(ManifestPath(), std::ios::out | std::ios::trunc);
        if (!out) return false;
        out << "level_scene=" << rel << "\n";
        return true;
    }
}
