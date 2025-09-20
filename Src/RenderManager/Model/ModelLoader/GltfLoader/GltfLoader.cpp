#include "GltfLoader.h"
#include <cassert>
#include <cstring>
#include <algorithm>

#define CGLTF_IMPLEMENTATION
#include "gltf/cgltf.h"

static inline DirectX::XMVECTOR DXLoad3(const DirectX::XMFLOAT3& v) { return DirectX::XMLoadFloat3(&v); }
static inline DirectX::XMVECTOR DXLoad2(const DirectX::XMFLOAT2& v) { return DirectX::XMLoadFloat2(&v); }

static inline void OrthonormalizeTangent(const DirectX::XMFLOAT3& n,
    DirectX::XMFLOAT3& t,
    DirectX::XMFLOAT3& b)
{
    using namespace DirectX;
    XMVECTOR N = XMLoadFloat3(&n);
    XMVECTOR T = XMLoadFloat3(&t);
    // Gram-Schmidt
    T = XMVector3Normalize(T - XMVector3Dot(T, N) * N);
    XMVECTOR B = XMVector3Normalize(XMVector3Cross(N, T));
    XMStoreFloat3(&t, T);
    XMStoreFloat3(&b, B);
}

static inline void FlipHandedness(DirectX::XMFLOAT3& p,
    DirectX::XMFLOAT3& n,
    DirectX::XMFLOAT3& t,
    DirectX::XMFLOAT3& b)
{
    p.z = -p.z; n.z = -n.z; t.z = -t.z; b.z = -b.z;
}

std::shared_ptr<MeshBuffer> GltfLoader::Load(const std::string& path,
    bool flipHandedness,
    bool flipV)
{
    auto parsed = ParseGLTFFile(path);
    if (!parsed.data || parsed.chosen.meshIndex == SIZE_MAX)
    {
        Free(parsed.data);
        return nullptr;
    }

    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT2> texcoords;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<DirectX::XMFLOAT4> tangents4;
    std::vector<uint32_t>          indices;

    if (!ExtractAttributes(parsed.data, parsed.chosen, positions, texcoords, normals, tangents4, indices))
    {
        Free(parsed.data);
        return nullptr;
    }

    if (flipV)
    {
        for (auto& uv : texcoords) uv.y = 1.0f - uv.y;
    }

    NormalizePositions(positions);

    std::vector<ModelVertex> verts;
    BuildVertices(positions, texcoords, normals, tangents4, indices, verts);

    if (flipHandedness)
    {
        for (auto& v : verts)
        {
            FlipHandedness(v.Position, v.Normal, v.Tangent, v.BiNormal);
            using namespace DirectX;
            auto norm = XMVector3Normalize(XMLoadFloat3(&v.Normal));
            XMStoreFloat3(&v.Normal, norm);
            auto tan = XMVector3Normalize(XMLoadFloat3(&v.Tangent));
            XMStoreFloat3(&v.Tangent, tan);
            auto bin = XMVector3Normalize(XMLoadFloat3(&v.BiNormal));
            XMStoreFloat3(&v.BiNormal, bin);
        }

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
            std::swap(indices[i + 1], indices[i + 2]);
    }

    auto mesh = std::make_shared<MeshBuffer>(verts, indices);
    Free(parsed.data);
    return mesh;
}

GltfLoader::ParsedGLTF GltfLoader::ParseGLTFFile(const std::string& path)
{
    ParsedGLTF out{};
    cgltf_options options = {};
    cgltf_data* data = nullptr;

    cgltf_result res = cgltf_parse_file(&options, path.c_str(), &data);
    if (res == cgltf_result_success)
        res = cgltf_load_buffers(&options, data, path.c_str());

    if (res != cgltf_result_success || !data)
    {
        if (data) cgltf_free(data);
        return out;
    }

    for (size_t m = 0; m < data->meshes_count; ++m)
    {
        const cgltf_mesh& mesh = data->meshes[m];
        for (size_t p = 0; p < mesh.primitives_count; ++p)
        {
            const cgltf_primitive& prim = mesh.primitives[p];
            bool hasPos = false, hasNrm = false, hasUV0 = false;
            for (size_t a = 0; a < prim.attributes_count; ++a)
            {
                const cgltf_attribute& at = prim.attributes[a];
                if (at.type == cgltf_attribute_type_position) hasPos = true;
                else if (at.type == cgltf_attribute_type_normal) hasNrm = true;
                else if (at.type == cgltf_attribute_type_texcoord && at.index == 0) hasUV0 = true;
            }
            if (hasPos && hasNrm && hasUV0)
            {
                out.data = data;
                out.chosen.meshIndex = m;
                out.chosen.primIndex = p;
                return out;
            }
        }
    }

    for (size_t m = 0; m < data->meshes_count; ++m)
    {
        const cgltf_mesh& mesh = data->meshes[m];
        if (mesh.primitives_count > 0)
        {
            out.data = data;
            out.chosen.meshIndex = m;
            out.chosen.primIndex = 0;
            return out;
        }
    }

    cgltf_free(data);
    out.data = nullptr;
    return out;
}

bool GltfLoader::ExtractAttributes(void* gltfData,
    const ParsedGLTF::PrimitiveRef& pick,
    std::vector<DirectX::XMFLOAT3>& positions,
    std::vector<DirectX::XMFLOAT2>& texcoords,
    std::vector<DirectX::XMFLOAT3>& normals,
    std::vector<DirectX::XMFLOAT4>& tangents,
    std::vector<uint32_t>& indices)
{
    if (!gltfData || pick.meshIndex == SIZE_MAX) return false;

    const cgltf_data* data = static_cast<const cgltf_data*>(gltfData);
    const cgltf_mesh& mesh = data->meshes[pick.meshIndex];
    const cgltf_primitive& prim = mesh.primitives[pick.primIndex];

    if (prim.indices && prim.indices->count > 0)
    {
        indices.resize(prim.indices->count);
        for (size_t i = 0; i < prim.indices->count; ++i)
        {
            const cgltf_uint idx = cgltf_accessor_read_index(prim.indices, i);
            indices[i] = static_cast<uint32_t>(idx);
        }
    }
    else
    {
        cgltf_size vcount = 0;
        for (cgltf_size a = 0; a < prim.attributes_count; ++a)
            vcount = std::max(vcount, prim.attributes[a].data->count);
        indices.resize(vcount);
        for (uint32_t i = 0; i < static_cast<uint32_t>(vcount); ++i) indices[i] = i;
    }

    cgltf_accessor* accPos = nullptr;
    cgltf_accessor* accNrm = nullptr;
    cgltf_accessor* accUV0 = nullptr;
    cgltf_accessor* accTan = nullptr;

    for (size_t a = 0; a < prim.attributes_count; ++a)
    {
        const cgltf_attribute& at = prim.attributes[a];
        if (at.type == cgltf_attribute_type_position) accPos = at.data;
        else if (at.type == cgltf_attribute_type_normal) accNrm = at.data;
        else if (at.type == cgltf_attribute_type_texcoord && at.index == 0) accUV0 = at.data;
        else if (at.type == cgltf_attribute_type_tangent) accTan = at.data; // xyz + sign
    }

    if (!accPos) return false;
    const size_t vcount = accPos->count;

    auto read_vec2 = [](cgltf_accessor* acc, size_t i) {
        float v[4] = { 0,0,0,0 };
        cgltf_accessor_read_float(acc, i, v, 2);
        return DirectX::XMFLOAT2(v[0], v[1]);
        };
    auto read_vec3 = [](cgltf_accessor* acc, size_t i) {
        float v[4] = { 0,0,0,0 };
        cgltf_accessor_read_float(acc, i, v, 3);
        return DirectX::XMFLOAT3(v[0], v[1], v[2]);
        };
    auto read_vec4 = [](cgltf_accessor* acc, size_t i) {
        float v[4] = { 0,0,0,0 };
        cgltf_accessor_read_float(acc, i, v, 4);
        return DirectX::XMFLOAT4(v[0], v[1], v[2], v[3]);
        };

    positions.resize(vcount);
    for (size_t i = 0; i < vcount; ++i) positions[i] = read_vec3(accPos, i);

    if (accNrm) {
        normals.resize(vcount);
        for (size_t i = 0; i < vcount; ++i) normals[i] = read_vec3(accNrm, i);
    }
    else {
        normals.assign(vcount, DirectX::XMFLOAT3(0, 0, 1));
    }

    if (accUV0) {
        texcoords.resize(vcount);
        for (size_t i = 0; i < vcount; ++i) texcoords[i] = read_vec2(accUV0, i);
    }
    else {
        texcoords.assign(vcount, DirectX::XMFLOAT2(0, 0));
    }

    tangents.clear();
    if (accTan) {
        tangents.resize(vcount);
        for (size_t i = 0; i < vcount; ++i) tangents[i] = read_vec4(accTan, i); // xyz + sign
    }

    return true;
}

void GltfLoader::BuildVertices(const std::vector<DirectX::XMFLOAT3>& positions,
    const std::vector<DirectX::XMFLOAT2>& texcoords,
    const std::vector<DirectX::XMFLOAT3>& normals,
    const std::vector<DirectX::XMFLOAT4>& tangents,
    const std::vector<uint32_t>& indices,
    std::vector<ModelVertex>& outVertices)
{
    const size_t vcount = positions.size();
    outVertices.resize(vcount);

    if (!tangents.empty())
    {
        for (size_t i = 0; i < vcount; ++i)
        {
            outVertices[i].Position = positions[i];
            outVertices[i].TexCoord = i < texcoords.size() ? texcoords[i] : DirectX::XMFLOAT2(0, 0);
            outVertices[i].Normal = i < normals.size() ? normals[i] : DirectX::XMFLOAT3(0, 0, 1);

            const auto& t4 = tangents[i]; // xyz + sign
            outVertices[i].Tangent = DirectX::XMFLOAT3(t4.x, t4.y, t4.z);

            using namespace DirectX;
            XMVECTOR N = XMLoadFloat3(&outVertices[i].Normal);
            XMVECTOR T = XMLoadFloat3(&outVertices[i].Tangent);
            XMVECTOR B = XMVector3Normalize(XMVector3Cross(N, T));
            if (t4.w < 0.0f) B = XMVectorNegate(B);
            XMStoreFloat3(&outVertices[i].BiNormal, B);

            OrthonormalizeTangent(outVertices[i].Normal, outVertices[i].Tangent, outVertices[i].BiNormal);
        }
    }
    else
    {
        std::vector<ModelVertex> temp(vcount);
        for (size_t i = 0; i < vcount; ++i)
        {
            temp[i].Position = positions[i];
            temp[i].TexCoord = i < texcoords.size() ? texcoords[i] : DirectX::XMFLOAT2(0, 0);
            temp[i].Normal = i < normals.size() ? normals[i] : DirectX::XMFLOAT3(0, 0, 1);
            temp[i].Tangent = DirectX::XMFLOAT3(0, 0, 0);
            temp[i].BiNormal = DirectX::XMFLOAT3(0, 0, 0);
        }
        ComputeTangents(temp, indices, outVertices);
    }
}

void GltfLoader::ComputeTangents(const std::vector<ModelVertex>& inVerts,
    const std::vector<uint32_t>& indices,
    std::vector<ModelVertex>& outVerts)
{
    using namespace DirectX;
    outVerts = inVerts;
    std::vector<XMFLOAT3> tanAccum(outVerts.size(), XMFLOAT3(0, 0, 0));
    std::vector<XMFLOAT3> bitAccum(outVerts.size(), XMFLOAT3(0, 0, 0));

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        uint32_t i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
        const auto& v0 = outVerts[i0];
        const auto& v1 = outVerts[i1];
        const auto& v2 = outVerts[i2];

        XMVECTOR p0 = XMLoadFloat3(&v0.Position);
        XMVECTOR p1 = XMLoadFloat3(&v1.Position);
        XMVECTOR p2 = XMLoadFloat3(&v2.Position);

        XMVECTOR w0 = XMLoadFloat2(&v0.TexCoord);
        XMVECTOR w1 = XMLoadFloat2(&v1.TexCoord);
        XMVECTOR w2 = XMLoadFloat2(&v2.TexCoord);

        XMVECTOR e1 = p1 - p0;
        XMVECTOR e2 = p2 - p0;

        XMFLOAT2 uv0, uv1, uv2;
        XMStoreFloat2(&uv0, w0);
        XMStoreFloat2(&uv1, w1);
        XMStoreFloat2(&uv2, w2);

        float x1 = uv1.x - uv0.x, x2 = uv2.x - uv0.x;
        float y1 = uv1.y - uv0.y, y2 = uv2.y - uv0.y;
        float denom = (x1 * y2 - x2 * y1);
        float r = (fabsf(denom) > 1e-20f) ? 1.0f / denom : 0.0f;

        XMVECTOR T = r * (y2 * e1 - y1 * e2);
        XMVECTOR B = r * (-x2 * e1 + x1 * e2);

        XMFLOAT3 Tf, Bf;
        XMStoreFloat3(&Tf, T);
        XMStoreFloat3(&Bf, B);

        auto accum = [&](uint32_t idx, const XMFLOAT3& t, const XMFLOAT3& b) {
            tanAccum[idx].x += t.x; tanAccum[idx].y += t.y; tanAccum[idx].z += t.z;
            bitAccum[idx].x += b.x; bitAccum[idx].y += b.y; bitAccum[idx].z += b.z;
            };
        accum(i0, Tf, Bf);
        accum(i1, Tf, Bf);
        accum(i2, Tf, Bf);
    }

    for (size_t i = 0; i < outVerts.size(); ++i)
    {
        outVerts[i].Tangent = tanAccum[i];
        outVerts[i].BiNormal = bitAccum[i];
        OrthonormalizeTangent(outVerts[i].Normal, outVerts[i].Tangent, outVerts[i].BiNormal);
    }
}

void GltfLoader::NormalizePositions(std::vector<DirectX::XMFLOAT3>& positions)
{
    if (positions.empty()) return;
    using namespace DirectX;

    XMFLOAT3 minB(FLT_MAX, FLT_MAX, FLT_MAX);
    XMFLOAT3 maxB(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (auto& p : positions)
    {
        minB.x = std::min(minB.x, p.x); minB.y = std::min(minB.y, p.y); minB.z = std::min(minB.z, p.z);
        maxB.x = std::max(maxB.x, p.x); maxB.y = std::max(maxB.y, p.y); maxB.z = std::max(maxB.z, p.z);
    }

    XMFLOAT3 center{ (minB.x + maxB.x) * 0.5f, (minB.y + maxB.y) * 0.5f, (minB.z + maxB.z) * 0.5f };
    float sx = maxB.x - minB.x, sy = maxB.y - minB.y, sz = maxB.z - minB.z;
    float scale = std::max({ sx, sy, sz });
    if (scale <= 1e-8f) scale = 1.0f;

    for (auto& p : positions)
    {
        p.x = (p.x - center.x) / scale;
        p.y = (p.y - center.y) / scale;
        p.z = (p.z - center.z) / scale;
    }
}

void GltfLoader::Free(void* gltfData)
{
    if (!gltfData) return;
    cgltf_free(static_cast<cgltf_data*>(gltfData));
}
