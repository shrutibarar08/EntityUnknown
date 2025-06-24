#include "ObjLoader.h"

#include <fstream>
#include <sstream>
#include <tuple>
#include <functional>
#include <unordered_map>
#include <array>


std::shared_ptr<MeshBuffer> ObjLoader::Load(const std::string& path)
{
    // Phase 1: Parse raw OBJ data
    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT2> texcoords;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<std::string> faceLines;

    if (!ParseOBJFile(path, positions, texcoords, normals, faceLines))
        return nullptr;

    NormalizePositions(positions);

    // Phase 2: Build unique vertices and indices
    std::vector<ModelVertex> outVertices;
    std::vector<uint32_t> outIndices;

    ProcessFaces(faceLines, positions, texcoords, normals, outVertices, outIndices);

    // Phase 3: Compute tangents and bi-normals for normal mapping
    ComputeTangents(outVertices, outIndices, outVertices);

    // Phase 4: Upload to GPU buffer
    auto buffer = std::make_shared<MeshBuffer>(outVertices, outIndices);
    return buffer;
}
bool ObjLoader::ParseOBJFile(
    const std::string& path,
    std::vector<DirectX::XMFLOAT3>& positions,
    std::vector<DirectX::XMFLOAT2>& texcoords,
    std::vector<DirectX::XMFLOAT3>& normals,
    std::vector<std::string>& faceLines)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") // Position
        {
            DirectX::XMFLOAT3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vt") // TexCoord
        {
            DirectX::XMFLOAT2 uv = { 0.0f, 0.0f };
            iss >> uv.x >> uv.y;
            uv.y = 1.0f - uv.y;
            texcoords.push_back(uv);
        }
        else if (prefix == "vn") // Normal
        {
            DirectX::XMFLOAT3 norm = { 0.0f, 0.0f, 1.0f };
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);
        }
        else if (prefix == "f")
        {
            faceLines.push_back(line);
        }
    }

    return true;
}

void ObjLoader::ProcessFaces(
    const std::vector<std::string>& faceLines,
    const std::vector<DirectX::XMFLOAT3>& positions,
    const std::vector<DirectX::XMFLOAT2>& texcoords,
    const std::vector<DirectX::XMFLOAT3>& normals,
    std::vector<ModelVertex>& outVertices,
    std::vector<uint32_t>& outIndices)
{
    std::unordered_map<VertexKey, uint32_t> uniqueMap;

    for (const std::string& faceLine : faceLines)
    {
        std::istringstream iss(faceLine);
        std::string prefix;
        iss >> prefix;

        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token)
            tokens.push_back(token);

        if (tokens.size() < 3)
            continue; // not a valid face

        for (size_t i = 1; i + 1 < tokens.size(); ++i)
        {
            std::array<std::string, 3> faceVerts = { tokens[0], tokens[i], tokens[i + 1] };

            for (const std::string& vertStr : faceVerts)
            {
                int pi = 0, ti = 0, ni = 0;
                sscanf_s(vertStr.c_str(), "%d/%d/%d", &pi, &ti, &ni);

                // Skip invalid indices
                if (pi <= 0 || pi > static_cast<int>(positions.size()) ||
                    ti <= 0 || ti > static_cast<int>(texcoords.size()) ||
                    ni <= 0 || ni > static_cast<int>(normals.size()))
                    continue;

                VertexKey key{ pi, ti, ni };
                auto it = uniqueMap.find(key);
                if (it != uniqueMap.end())
                {
                    outIndices.push_back(it->second);
                    continue;
                }

                ModelVertex vertex;
                vertex.Position = positions[pi - 1];
                vertex.TexCoord = texcoords[ti - 1];
                vertex.Normal = normals[ni - 1];
                vertex.Tangent = { 0.0f, 0.0f, 0.0f };
                vertex.BiNormal = { 0.0f, 0.0f, 0.0f };

                uint32_t index = static_cast<uint32_t>(outVertices.size());
                uniqueMap[key] = index;
                outVertices.push_back(vertex);
                outIndices.push_back(index);
            }
        }
    }
}

void ObjLoader::ComputeTangents(
    const std::vector<ModelVertex>& inVerts,
    const std::vector<uint32_t>& indices,
    std::vector<ModelVertex>& outVerts)
{
    const size_t vertexCount = inVerts.size();
    std::vector<DirectX::XMFLOAT3> accumulatedTangents(vertexCount, { 0, 0, 0 });
    std::vector<DirectX::XMFLOAT3> accumulatedBinormals(vertexCount, { 0, 0, 0 });

    using namespace DirectX;

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        const auto& v0 = inVerts[i0];
        const auto& v1 = inVerts[i1];
        const auto& v2 = inVerts[i2];

        XMVECTOR p0 = XMLoadFloat3(&v0.Position);
        XMVECTOR p1 = XMLoadFloat3(&v1.Position);
        XMVECTOR p2 = XMLoadFloat3(&v2.Position);

        XMVECTOR uv0 = XMLoadFloat2(&v0.TexCoord);
        XMVECTOR uv1 = XMLoadFloat2(&v1.TexCoord);
        XMVECTOR uv2 = XMLoadFloat2(&v2.TexCoord);

        XMVECTOR dp1 = XMVectorSubtract(p1, p0);
        XMVECTOR dp2 = XMVectorSubtract(p2, p0);

        XMVECTOR duv1 = XMVectorSubtract(uv1, uv0);
        XMVECTOR duv2 = XMVectorSubtract(uv2, uv0);

        float du1 = XMVectorGetX(duv1), dv1 = XMVectorGetY(duv1);
        float du2 = XMVectorGetX(duv2), dv2 = XMVectorGetY(duv2);

        float det = du1 * dv2 - du2 * dv1;
        if (fabs(det) < 1e-6f)
            continue; // Skip degenerate UVs

        float invDet = 1.0f / det;

        XMVECTOR tangent = XMVectorScale(
            XMVectorSubtract(XMVectorScale(dp1, dv2), XMVectorScale(dp2, dv1)), invDet);

        XMVECTOR bitangent = XMVectorScale(
            XMVectorSubtract(XMVectorScale(dp2, du1), XMVectorScale(dp1, du2)), invDet);

        XMFLOAT3 t, b;
        XMStoreFloat3(&t, tangent);
        XMStoreFloat3(&b, bitangent);

        for (uint32_t idx : { i0, i1, i2 })
        {
            accumulatedTangents[idx].x += t.x;
            accumulatedTangents[idx].y += t.y;
            accumulatedTangents[idx].z += t.z;

            accumulatedBinormals[idx].x += b.x;
            accumulatedBinormals[idx].y += b.y;
            accumulatedBinormals[idx].z += b.z;
        }
    }

    outVerts = inVerts;
    for (size_t i = 0; i < vertexCount; ++i)
    {
        XMVECTOR normal = XMLoadFloat3(&inVerts[i].Normal);
        XMVECTOR tangent = XMLoadFloat3(&accumulatedTangents[i]);
        XMVECTOR bitangent = XMLoadFloat3(&accumulatedBinormals[i]);

        // Orthonormalize: Gram-Schmidt
        tangent = XMVector3Normalize(XMVectorSubtract(tangent, XMVectorScale(normal, XMVector3Dot(normal, tangent).m128_f32[0])));
        bitangent = XMVector3Normalize(bitangent);

        XMStoreFloat3(&outVerts[i].Tangent, tangent);
        XMStoreFloat3(&outVerts[i].BiNormal, bitangent);
    }
}

void ObjLoader::NormalizePositions(std::vector<DirectX::XMFLOAT3>& positions)
{
    using namespace DirectX;

    if (positions.empty()) return;

    XMFLOAT3 minPos = positions[0];
    XMFLOAT3 maxPos = positions[0];

    // Compute AABB (min/max bounds)
    for (size_t i = 1; i < positions.size(); ++i)
    {
        const auto& p = positions[i];

        if (p.x < minPos.x) minPos.x = p.x;
        if (p.y < minPos.y) minPos.y = p.y;
        if (p.z < minPos.z) minPos.z = p.z;

        if (p.x > maxPos.x) maxPos.x = p.x;
        if (p.y > maxPos.y) maxPos.y = p.y;
        if (p.z > maxPos.z) maxPos.z = p.z;
    }

    // Compute center
    XMFLOAT3 center = {
        (minPos.x + maxPos.x) * 0.5f,
        (minPos.y + maxPos.y) * 0.5f,
        (minPos.z + maxPos.z) * 0.5f
    };

    // Compute maximum extent (largest dimension)
    float extentX = maxPos.x - minPos.x;
    float extentY = maxPos.y - minPos.y;
    float extentZ = maxPos.z - minPos.z;
    float maxExtent = extentX;

    if (extentY > maxExtent) maxExtent = extentY;
    if (extentZ > maxExtent) maxExtent = extentZ;

    if (maxExtent < 1e-6f) return; // Avoid divide-by-zero

    float invExtent = 1.0f / maxExtent;

    // Normalize each position to fit within unit cube centered at origin
    for (auto& pos : positions)
    {
        pos.x = (pos.x - center.x) * invExtent;
        pos.y = (pos.y - center.y) * invExtent;
        pos.z = (pos.z - center.z) * invExtent;
    }
}
