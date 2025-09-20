#include "ModelLoader.h"

#include "ObjLoader/ObjLoader.h"
#include "GltfLoader/GltfLoader.h"
#include "Utils/Logger/Logger.h"

std::shared_ptr<MeshBuffer> ModelLoader::LoadModel(const std::string& path)
{
    if (m_ModelCache.contains(path))
    {
        LOG_INFO("Cache Found for the mesh: " + path);
        return m_ModelCache[path];
    }

    auto try_cache = [&](std::shared_ptr<MeshBuffer> buffer) -> std::shared_ptr<MeshBuffer>
        {
            if (!buffer)
            {
                LOG_ERROR("Failed to build mesh for: " + path);
                return nullptr;
            }
            LOG_INFO("Cache not found for the mesh: " + path);
            m_ModelCache[path] = std::move(buffer);
            if (m_ModelCache[path] == nullptr)
            {
                LOG_ERROR("Failed to build mesh (null after move): " + path);
            }
            else
            {
                LOG_SUCCESS("Build Success!");
            }
            return m_ModelCache[path];
        };

    if (path.ends_with(".obj"))
    {
        return try_cache(ObjLoader::Load(path));
    }
    else if (path.ends_with(".gltf") || path.ends_with(".glb"))
    {
        return try_cache(GltfLoader::Load(path, true, false));
    }

    LOG_ERROR("Unsupported model extension for: " + path);
    return nullptr;
}

std::shared_ptr<MeshBuffer> ModelLoader::LoadFromDisk(const std::string& path)
{
    if (path.ends_with(".obj"))
    {
        return ObjLoader::Load(path);
    }
    else if (path.ends_with(".gltf") || path.ends_with(".glb"))
    {
        return GltfLoader::Load(path, true, false);
    }

    LOG_ERROR("Unsupported model extension for: " + path);
    return nullptr;
}
