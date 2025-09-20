#pragma once
#include <memory>
#include <string>
#include <vector>
#include <DirectXMath.h>

#include "RenderManager/Model/ModelLoader/ModelMeta.h"


class GltfLoader
{
public:
    static std::shared_ptr<MeshBuffer> Load(const std::string& path,
        bool flipHandedness = false,
        bool flipV = false);

    struct ParsedGLTF {
        struct PrimitiveRef {
            size_t meshIndex = SIZE_MAX;
            size_t primIndex = SIZE_MAX;
        };
        void* data = nullptr;
        PrimitiveRef   chosen;
    };

    static ParsedGLTF ParseGLTFFile(const std::string& path);

    static bool ExtractAttributes(void* gltfData,
        const ParsedGLTF::PrimitiveRef& pick,
        std::vector<DirectX::XMFLOAT3>& positions,
        std::vector<DirectX::XMFLOAT2>& texcoords,
        std::vector<DirectX::XMFLOAT3>& normals,
        std::vector<DirectX::XMFLOAT4>& tangents, // xyz + sign (w)
        std::vector<uint32_t>& indices);

    static void BuildVertices(const std::vector<DirectX::XMFLOAT3>& positions,
        const std::vector<DirectX::XMFLOAT2>& texcoords,
        const std::vector<DirectX::XMFLOAT3>& normals,
        const std::vector<DirectX::XMFLOAT4>& tangents,
        const std::vector<uint32_t>& indices,
        std::vector<ModelVertex>& outVertices);

    static void ComputeTangents(const std::vector<ModelVertex>& inVerts,
        const std::vector<uint32_t>& indices,
        std::vector<ModelVertex>& outVerts);

    static void NormalizePositions(std::vector<DirectX::XMFLOAT3>& positions);

    static void Free(void* gltfData);
};
