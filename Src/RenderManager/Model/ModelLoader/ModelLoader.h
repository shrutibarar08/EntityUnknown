#pragma once
#include <unordered_map>
#include "ModelMeta.h"


class ModelLoader
{
public:
	ModelLoader() = default;
	~ModelLoader() = default;

	ModelLoader(const ModelLoader&) = delete;
	ModelLoader(ModelLoader&&) = delete;
	ModelLoader& operator=(const ModelLoader&) = delete;
	ModelLoader& operator=(ModelLoader&&) = delete;

	static std::shared_ptr<MeshBuffer> LoadModel(const std::string& path);
	static std::shared_ptr<MeshBuffer> LoadFromDisk(const std::string& path);

private:
	inline static std::unordered_map<std::string, std::shared_ptr<MeshBuffer>> m_ModelCache{};
};
