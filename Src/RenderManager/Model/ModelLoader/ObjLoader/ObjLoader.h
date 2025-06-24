#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>

#include "RenderManager/Model/ModelLoader/ModelMeta.h"

struct VertexKey
{
    int posIdx;
    int texIdx;
    int normIdx;

    bool operator==(const VertexKey& other) const noexcept
    {
        return posIdx == other.posIdx &&
            texIdx == other.texIdx &&
            normIdx == other.normIdx;
    }
};

namespace std
{
    template <>
    struct hash<VertexKey>
    {
        size_t operator()(const VertexKey& key) const noexcept
        {
            size_t h1 = std::hash<int>()(key.posIdx);
            size_t h2 = std::hash<int>()(key.texIdx);
            size_t h3 = std::hash<int>()(key.normIdx);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

class ObjLoader
{
public:
    static std::shared_ptr<MeshBuffer> Load(const std::string& path);

    static bool ParseOBJFile(
        const std::string& path,
        std::vector<DirectX::XMFLOAT3>& positions,
        std::vector<DirectX::XMFLOAT2>& texcoords,
        std::vector<DirectX::XMFLOAT3>& normals,
        std::vector<std::string>& faceLines);

    static void ProcessFaces(
        const std::vector<std::string>& faceLines,
        const std::vector<DirectX::XMFLOAT3>& positions,
        const std::vector<DirectX::XMFLOAT2>& texcoords,
        const std::vector<DirectX::XMFLOAT3>& normals,
        std::vector<ModelVertex>& outVertices,
        std::vector<uint32_t>& outIndices);

    static void ComputeTangents(
        const std::vector<ModelVertex>& inVerts,
        const std::vector<uint32_t>& indices,
        std::vector<ModelVertex>& outVerts);

    static void NormalizePositions(std::vector<DirectX::XMFLOAT3>& positions);
};