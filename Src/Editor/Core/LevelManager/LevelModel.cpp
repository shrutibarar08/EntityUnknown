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
}

void Level::UnHook()
{
	if (m_eLevelState == LevelState::NOT_HOOKED) return;
	m_eLevelState = LevelState::NOT_HOOKED;

	OffLoadLights();
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

void Level::LoadLevelSaveData(const nlohmann::json& levelData)
{
	LoadLightSaveData(levelData["Lights"]);
}

nlohmann::json Level::GetLevelSaveData() const
{
	nlohmann::json data{};
	data["Lights"] = std::move(GetLightSaveData());
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
		if (mesh) continue;
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
		if (mesh) continue;
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
