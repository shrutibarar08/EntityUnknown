#include "LevelModel.h"
#include <utility>
#include <assert.h>

#include "RenderManager/RenderQueue/RenderQueue.h"

#include "SystemManager/Registry/RegistryLight.h"

void Level::Hook()
{
	if (m_eLevelState == LevelState::HOOKED) return;
	m_eLevelState = LevelState::HOOKED;

	UploadLights();
	UploadMeshes();
	UploadBackgroundSprites();
	UploadFrontSprites();
}

void Level::UnHook()
{
	if (m_eLevelState == LevelState::NOT_HOOKED) return;
	m_eLevelState = LevelState::NOT_HOOKED;

	OffLoadLights();
	OffLoadMeshes();
	OffLoadBackgroundSprites();
	OffLoadFrontSprites();
}

void Level::AddLight(std::unique_ptr<ILightSource> light)
{
	ID lightId = light->GetAssignedID();
	m_mapLights[lightId] = { std::move(light), true };
	m_bDirtyLight = true;
	UploadLight(lightId);
}

bool Level::RemoveLight(ILightSource* lightSource) 
{
	if (not lightSource) return false;
	return RemoveLight(lightSource->GetAssignedID());
}

bool Level::RemoveLight(ID lightId)
{
	if (!IsAValidLight(lightId)) return false;
	OffLoadLight(lightId);
	m_mapLights.erase(lightId);	
	m_bDirtyLight = true;
	return true;
}

void Level::TurnOffLight(ID lightId)
{
	if (!IsAValidLight(lightId)) return;
	m_mapLights[lightId].TurnOn = false;

	OffLoadLight(lightId);
}

void Level::TurnONLight(ID lightId)
{
	if (!IsAValidLight(lightId)) return;
	m_mapLights[lightId].TurnOn = true;

	UploadLight(lightId);
}

bool Level::IsLightOn(ID lightId)
{
	return false;
}

std::unique_ptr<ILightSource> Level::RemoveAndGetLight(ID lightId)
{
	if (!IsAValidLight(lightId)) return nullptr;
	
	OffLoadLight(lightId);

	auto light = std::move(m_mapLights[lightId].Light);
	m_mapLights.erase(lightId);
	m_bDirtyLight = true;
	return light;
}

bool Level::IsAValidLight(ID lightId)
{
	return m_mapLights.contains(lightId);
}

const std::unordered_map<ID, ILightSource*>& Level::GetLightMap()
{
	if (m_bDirtyLight) RebuildSafeLights();
	return m_safeMapLights;
}

bool Level::AddMesh(std::unique_ptr<IRender> mesh)
{
	if (!mesh) return false;
	ID meshId = mesh->GetAssignedID();
	m_mapMeshes[meshId] = std::move(mesh);
	UploadMesh(meshId);
	m_bDirtyMesh = true;

	return true;
}

bool Level::RemoveMesh(IRender* mesh)
{
	if (!mesh) return false;
	return RemoveMesh(mesh->GetAssignedID());
}

bool Level::RemoveMesh(ID meshId)
{
	if (!IsAValidMesh(meshId)) return false;
	if (IsHooked()) RenderQueue::Get()->RemoveRender(meshId);
	
	m_bDirtyMesh = true;

	m_mapMeshes.erase(meshId);
	return true;
}

bool Level::IsAValidMesh(ID meshId)
{
	return m_mapMeshes.contains(meshId);
}

IRender* Level::GetMesh(ID meshId)
{
	if (!IsAValidMesh(meshId)) return nullptr;
	return m_mapMeshes[meshId].get();
}

std::unique_ptr<IRender> Level::RemoveAndGetMesh(ID meshId)
{
	if (not m_mapMeshes.contains(meshId)) return nullptr;

	OffLoadMesh(meshId);

	auto mesh = std::move(m_mapMeshes[meshId]);
	m_mapMeshes.erase(meshId);
	m_bDirtyMesh = true;
	return std::move(mesh);
}

const std::unordered_map<ID, IRender*>& Level::GetMeshMap()
{
	if (m_bDirtyMesh) RebuildSafeMeshes();
	return m_safeMapMeshes;
}

// Background Sprites

bool Level::AddBackgroundSprite(std::unique_ptr<IRender> sprite)
{
	if (!sprite) return false;
	const ID sid = sprite->GetAssignedID();
	m_mapBackgroundSprite[sid] = std::move(sprite);
	UploadBackgroundSprite(sid);
	m_bDirtyBackgroundSprite = true;
	return true;
}

bool Level::RemoveBackgroundSprite(IRender* sprite)
{
	if (!sprite) return false;
	return RemoveBackgroundSprite(sprite->GetAssignedID());
}

bool Level::RemoveBackgroundSprite(ID spriteId)
{
	if (!IsAValidBackgroundSprite(spriteId)) return false;
	if (IsHooked()) RenderQueue::Get()->RemoveRender(spriteId);

	m_mapBackgroundSprite.erase(spriteId);
	m_bDirtyBackgroundSprite = true;
	return true;
}

bool Level::IsAValidBackgroundSprite(ID spriteId)
{
	return m_mapBackgroundSprite.contains(spriteId);
}

IRender* Level::GetBackgroundSprite(ID spriteId)
{
	if (!IsAValidBackgroundSprite(spriteId)) return nullptr;
	return m_mapBackgroundSprite[spriteId].get();
}

std::unique_ptr<IRender> Level::RemoveAndGetBackgroundSprite(ID spriteId)
{
	if (!m_mapBackgroundSprite.contains(spriteId)) return nullptr;

	OffLoadBackgroundSprite(spriteId);

	auto uptr = std::move(m_mapBackgroundSprite[spriteId]);
	m_mapBackgroundSprite.erase(spriteId);
	m_bDirtyBackgroundSprite = true;
	return uptr;
}

const std::unordered_map<ID, IRender*>& Level::GetBackgroundSpriteMap()
{
	if (m_bDirtyBackgroundSprite) RebuildSafeBackgroundSprites();
	return m_safeBackgroundSprite;
}

void Level::RebuildSafeBackgroundSprites()
{
	m_safeBackgroundSprite.clear();
	for (auto& [id, up] : m_mapBackgroundSprite)
		m_safeBackgroundSprite[id] = up.get();
	m_bDirtyBackgroundSprite = false;
}

// Front Sprites

bool Level::AddFrontSprite(std::unique_ptr<IRender> sprite)
{
	if (!sprite) return false;
	const ID sid = sprite->GetAssignedID();
	m_mapFrontSprite[sid] = std::move(sprite);
	UploadFrontSprite(sid);
	m_bDirtyFrontSprite = true;
	return true;
}

bool Level::RemoveFrontSprite(IRender* sprite)
{
	if (!sprite) return false;
	return RemoveFrontSprite(sprite->GetAssignedID());
}

bool Level::RemoveFrontSprite(ID spriteId)
{
	if (!IsAValidFrontSprite(spriteId)) return false;
	if (IsHooked()) RenderQueue::Get()->RemoveRender(spriteId);

	m_mapFrontSprite.erase(spriteId);
	m_bDirtyFrontSprite = true;
	return true;
}

bool Level::IsAValidFrontSprite(ID spriteId)
{
	return m_mapFrontSprite.contains(spriteId);
}

IRender* Level::GetFrontSprite(ID spriteId)
{
	if (!IsAValidFrontSprite(spriteId)) return nullptr;
	return m_mapFrontSprite[spriteId].get();
}

std::unique_ptr<IRender> Level::RemoveAndGetFrontSprite(ID spriteId)
{
	if (!m_mapFrontSprite.contains(spriteId)) return nullptr;

	OffLoadFrontSprite(spriteId);

	auto uptr = std::move(m_mapFrontSprite[spriteId]);
	m_mapFrontSprite.erase(spriteId);
	m_bDirtyFrontSprite = true;
	return uptr;
}

const std::unordered_map<ID, IRender*>& Level::GetFrontSpriteMap()
{
	if (m_bDirtyFrontSprite) RebuildSafeFrontSprites();
	return m_safeFrontSprite;
}

void Level::RebuildSafeFrontSprites()
{
	m_safeFrontSprite.clear();
	for (auto& [id, up] : m_mapFrontSprite)
		m_safeFrontSprite[id] = up.get();
	m_bDirtyFrontSprite = false;
}

void Level::LoadLevelSaveData(const nlohmann::json& levelData)
{
	if (!levelData.is_object()) return;

	if (levelData.contains("Lights") && levelData["Lights"].is_object())
		LoadLightSaveData(levelData["Lights"]);
	
	if (levelData.contains("Meshes") && levelData["Meshes"].is_object())
		LoadMeshSaveData(levelData["Meshes"]);
}

nlohmann::json Level::GetLevelSaveData() const
{
	nlohmann::json data{};
	data["Lights"] = std::move(GetLightSaveData());
	data["Meshes"] = std::move(GetMeshSaveData());
	return data;
}

ILightSource* Level::GetLight(ID lightId)
{
	if (m_bDirtyLight) RebuildSafeLights();
	if (not m_safeMapLights.contains(lightId)) return nullptr;
	return m_safeMapLights[lightId];
}

void Level::UploadLights()
{
	for (auto& [id, light] : m_mapLights)
	{
		UploadLight(id);
	}
}

void Level::UploadLight(ID lightId)
{
	if (not m_mapLights.contains(lightId))    return;
	if (not m_mapLights[lightId].Light.get()) return;
	if (not m_mapLights[lightId].TurnOn)	  return;

	if (!IsHooked()) return;

	RenderQueue::Get()->AddLight(m_mapLights[lightId].Light.get());
}

void Level::OffLoadLights()
{
	for (auto& [id, light] : m_mapLights)
	{
		OffLoadLight(id);
	}
}

void Level::OffLoadLight(ID lightId)
{
	if (!IsAValidLight(lightId)) return;
	RenderQueue::Get()->RemoveLight(lightId);
}

void Level::RebuildSafeLights()
{
	if (!m_bDirtyLight) return;
	m_bDirtyLight = false;

	m_safeMapLights.clear();

	for (auto& [id, light] : m_mapLights)
	{
		m_safeMapLights[id] = light.Light.get();
	}
}

void Level::UploadMeshes()
{
	for (auto& [id, mesh] : m_mapMeshes)
	{
		if (!mesh) continue;
		UploadMesh(id);
	}
}

void Level::UploadMesh(ID meshId)
{
	if (!IsAValidMesh(meshId)) return;
	if (IsHooked()) RenderQueue::Get()->AddRender(m_mapMeshes[meshId].get());
}

void Level::OffLoadMeshes()
{
	for (auto& [id, mesh] : m_mapMeshes)
	{
		if (!mesh) continue;
		OffLoadMesh(id);
	}
}

void Level::OffLoadMesh(ID meshId)
{
	if (!IsAValidMesh(meshId)) return;
	RenderQueue::Get()->RemoveRender(m_mapMeshes[meshId].get());
}

void Level::RebuildSafeMeshes()
{
	if (!m_bDirtyMesh) return;

	m_safeMapMeshes.clear();
	for (auto& [id, mesh] : m_mapMeshes)
	{
		m_safeMapMeshes[id] = mesh.get();
	}
	m_bDirtyMesh = false;
}

void Level::UploadBackgroundSprites()
{
	if (!IsHooked()) return;
	for (auto& [id, uptr] : m_mapBackgroundSprite)
	{
		UploadBackgroundSprite(id);
	}
}

void Level::UploadBackgroundSprite(ID lightId)
{
	if (!IsHooked()) return;
	auto it = m_mapBackgroundSprite.find(lightId);
	if (it == m_mapBackgroundSprite.end() || !it->second) return;

	RenderQueue::Get()->AddRenderBackground(it->second.get());
}

void Level::OffLoadBackgroundSprites()
{
	if (!IsHooked()) return;
	for (const auto& [id, _] : m_mapBackgroundSprite)
	{
		OffLoadBackgroundSprite(id);
	}
}

void Level::OffLoadBackgroundSprite(ID lightId)
{
	if (!IsHooked()) return;
	if (!m_mapBackgroundSprite.contains(lightId)) return;

	RenderQueue::Get()->RemoveRenderBackground(lightId);
}

void Level::UploadFrontSprites()
{
	if (!IsHooked()) return;
	for (auto& [id, uptr] : m_mapFrontSprite)
	{
		UploadFrontSprite(id);
	}
}

void Level::UploadFrontSprite(ID meshId)
{
	if (!IsHooked()) return;
	auto it = m_mapFrontSprite.find(meshId);
	if (it == m_mapFrontSprite.end() || !it->second) return;

	RenderQueue::Get()->AddRenderFront(it->second.get());
}

void Level::OffLoadFrontSprites()
{
	if (!IsHooked()) return;
	for (const auto& [id, _] : m_mapFrontSprite)
	{
		OffLoadFrontSprite(id);
	}
}

void Level::OffLoadFrontSprite(ID meshId)
{
	if (!IsHooked()) return;
	if (!m_mapFrontSprite.contains(meshId)) return;

	RenderQueue::Get()->RemoveRenderFront(meshId);
}

void Level::LoadLightSaveData(const nlohmann::json& levelData)
{
	if (!levelData.is_object()) return;

	for (const auto& [idStr, jLight] : levelData.items())
	{
		const std::string type = jLight.value("Type", std::string{});
		if (type.empty()) continue;

		std::unique_ptr<ILightSource> light = RegistryLight::CreateLight(type);
		if (!light) continue;

		light->LoadLightSaveData(jLight);

		const ID newId = light->GetAssignedID();
		AddLight(std::move(light));
		const bool turnOn = jLight.value("TurnOn", true);
		m_mapLights[newId].TurnOn = turnOn;
	}
	RebuildSafeLights();
}

nlohmann::json Level::GetLightSaveData() const
{
	nlohmann::json levelData = nlohmann::json::object();

	for (const auto& [id, light] : m_mapLights)
	{
		if (!light.Light) continue;

		nlohmann::json j = light.Light->GetLightSaveData();
		j["Type"] = light.Light->GetLightTypeToString();
		j["TurnOn"] = light.TurnOn;

		levelData[std::to_string(id)] = std::move(j);
	}

	return levelData;
}

void Level::LoadMeshSaveData(const nlohmann::json& levelData)
{
	if (!levelData.is_object()) return;

	for (auto it = levelData.begin(); it != levelData.end(); ++it)
	{
		const nlohmann::json& jMesh = it.value();
		if (!jMesh.is_object()) continue;

		const std::string type = jMesh.value("Type", std::string{});
		if (type.empty()) continue;

		std::unique_ptr<IRender> mesh = RegistryMesh::CreateMesh(type);
		if (!mesh) continue;

		mesh->LoadRenderSaveData(jMesh);
		const ID newId = mesh->GetAssignedID();

		if (type == "BackgroundSprite")
		{
			AddBackgroundSprite(std::move(mesh));
			m_bDirtyBackgroundSprite = true;
			RebuildSafeBackgroundSprites();
		}
		else if (type == "ScreenSprite")
		{
			AddFrontSprite(std::move(mesh));
			m_bDirtyFrontSprite = true;
			RebuildSafeFrontSprites();
		}
		else
		{
			AddMesh(std::move(mesh));
			m_bDirtyMesh = true;
			RebuildSafeMeshes();
		}

		(void)newId;
	}
}

nlohmann::json Level::GetMeshSaveData() const
{
	nlohmann::json out = nlohmann::json::object();

	for (const auto& [id, meshPtr] : m_mapMeshes)
	{
		if (!meshPtr) continue;
		nlohmann::json j = meshPtr->GetRenderSaveData();

		std::string typeStr;
		typeStr = meshPtr->GetTypeName();

		if (!typeStr.empty()) j["Type"] = typeStr;

		out[std::to_string(id)] = std::move(j);
	}

	for (const auto& [id, meshPtr] : m_mapBackgroundSprite)
	{
		if (!meshPtr) continue;
		nlohmann::json j = meshPtr->GetRenderSaveData();

		std::string typeStr;
		typeStr = meshPtr->GetTypeName();

		if (!typeStr.empty()) j["Type"] = typeStr;

		out[std::to_string(id)] = std::move(j);
	}

	for (const auto& [id, meshPtr] : m_mapFrontSprite)
	{
		if (!meshPtr) continue;
		nlohmann::json j = meshPtr->GetRenderSaveData();

		std::string typeStr;
		typeStr = meshPtr->GetTypeName();

		if (!typeStr.empty()) j["Type"] = typeStr;

		out[std::to_string(id)] = std::move(j);
	}

	return out;
}
