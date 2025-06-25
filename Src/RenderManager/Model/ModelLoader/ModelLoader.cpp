#include "ModelLoader.h"

#include "ObjLoader/ObjLoader.h"
#include "Utils/Logger/Logger.h"

std::shared_ptr<MeshBuffer> ModelLoader::LoadModel(const std::string& path)
{
	if (m_ModelCache.contains(path))
	{
		LOG_INFO("Cache Found for the mesh: " + path);
		return m_ModelCache[path];
	}

	if (path.ends_with(".obj"))
	{
		if (auto buffer = ObjLoader::Load(path))
		{
			LOG_INFO("Cache not found for the mesh: " + path);
			m_ModelCache[path] = std::move(buffer);
			if (m_ModelCache[path] == nullptr)
			{
				LOG_ERROR("Failed to build mesh");
			}
			else LOG_SUCCESS("Build Success!");
			return m_ModelCache[path];
		}
	}
	return nullptr;
}

std::shared_ptr<MeshBuffer> ModelLoader::LoadFromDisk(const std::string& path)
{
	if (path.ends_with(".obj"))
	{
		return ObjLoader::Load(path);
	}
	return nullptr;
}
