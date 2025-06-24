#pragma once

#include "RenderManager/Components/ModelBuffer.h"

struct ModelVertex
{
	DirectX::XMFLOAT3 Position;     // World/Model space position
	DirectX::XMFLOAT2 TexCoord;     // UV mapping
	DirectX::XMFLOAT3 Normal;       // Surface normal
	DirectX::XMFLOAT3 Tangent;      // Tangent for normal mapping
	DirectX::XMFLOAT3 BiNormal;     // Binormal for TBN matrix (optional, can be computed)
};
using DEFAULT_VERTEX = ModelVertex;
using MeshBuffer = StaticModelBufferSource<DEFAULT_VERTEX, uint32_t>;


typedef struct SUB_MESH_DATA
{
    std::string SubMeshName;
    std::shared_ptr<MeshBuffer> MeshBuffer;

    bool operator==(const SUB_MESH_DATA& other) const noexcept
    {
        return SubMeshName == other.SubMeshName;
    }
}SUB_MESH_DATA;

typedef struct MESH_DATA
{
    std::string MeshName;
    std::vector<SUB_MESH_DATA> MeshData;

    bool operator==(const MESH_DATA& other) const noexcept
    {
        return MeshName == other.MeshName;
    }
}MESH_DATA;

namespace std
{
    template <>
    struct hash<MESH_DATA>
    {
        std::size_t operator()(const MESH_DATA& data) const noexcept
        {
            return std::hash<std::string>{}(data.MeshName);
        }
    };
}
