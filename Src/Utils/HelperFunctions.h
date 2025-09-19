#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>
#include <filesystem>
#include <fstream>

#include <DirectXMath.h>
#include "nlohmann/json.hpp"

// -------------------------------------------------------------
// UTF conversion
// -------------------------------------------------------------
inline std::string WideToUtf8(const std::wstring& ws)
{
#if defined(_WIN32)
    if (ws.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    std::string s; s.resize(n);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), s.data(), n, nullptr, nullptr);
    return s;
#else
    return std::string(ws.begin(), ws.end());
#endif
}

// -------------------------------------------------------------
// JSON helpers for DirectX types
// -------------------------------------------------------------
inline nlohmann::json ToJson(const DirectX::XMFLOAT3& v)
{
    return nlohmann::json::array({ v.x, v.y, v.z });
}
inline nlohmann::json ToJson(const DirectX::XMFLOAT4& v)
{
    return nlohmann::json::array({ v.x, v.y, v.z, v.w });
}
inline nlohmann::json ToJson(const DirectX::XMMATRIX& M)
{
    DirectX::XMFLOAT4X4 f4x4{};
    DirectX::XMStoreFloat4x4(&f4x4, M);
    return nlohmann::json::array({
        nlohmann::json::array({ f4x4._11, f4x4._12, f4x4._13, f4x4._14 }),
        nlohmann::json::array({ f4x4._21, f4x4._22, f4x4._23, f4x4._24 }),
        nlohmann::json::array({ f4x4._31, f4x4._32, f4x4._33, f4x4._34 }),
        nlohmann::json::array({ f4x4._41, f4x4._42, f4x4._43, f4x4._44 })
        });
}

// -------------------------------------------------------------
// compile-time helpers
// -------------------------------------------------------------
template<class...Ts> struct type_list {};
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

// -------------------------------------------------------------
// Storage helpers
// -------------------------------------------------------------
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
    inline std::string DefaultLevelSceneRel() { return "DefaultLevel.scene"; }

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
