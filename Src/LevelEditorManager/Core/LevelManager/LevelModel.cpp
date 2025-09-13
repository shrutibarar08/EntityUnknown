#include "LevelModel.h"
#include <utility>
#include <assert.h>

#include "RenderManager/RenderQueue/RenderQueue.h"

void Level::Load()
{
	if (m_eLevelState == LevelState::LOADED) return;
	m_eLevelState = LevelState::LOADED;

	LoadLights();
}

void Level::UnLoad()
{
	if (m_eLevelState == LevelState::UNLOADED) return;
	m_eLevelState = LevelState::UNLOADED;

	UnLoadLights();
}

void Level::AddLight(std::unique_ptr<ILightSource> light)
{
	ID lightId = light->GetAssignedID();
	m_mapLights[lightId] = { std::move(light), true };
	m_bDirtyLight = true;
	LoadLight(lightId);
}

bool Level::RemoveLight(ILightSource* lightSource) 
{
	if (not lightSource) return false;
	return RemoveLight(lightSource->GetAssignedID());
}

bool Level::RemoveLight(ID lightId)
{
	if (not m_mapLights.contains(lightId)) return false;
	UnLoadLight(lightId);
	m_mapLights.erase(lightId);	
	m_bDirtyLight = true;
	return true;
}

std::unique_ptr<ILightSource> Level::RemoveAndGetLight(ID lightId)
{
	if (not m_mapLights.contains(lightId)) return nullptr;
	
	UnLoadLight(lightId);

	auto light = std::move(m_mapLights[lightId].Light);
	m_mapLights.erase(lightId);
	m_bDirtyLight = true;
	return light;
}

void Level::TurnOffLight(ID lightId)
{
	if (not m_mapLights.contains(lightId)) return;
	if (not m_mapLights[lightId].TurnOn) return;

	m_mapLights[lightId].TurnOn = false;
	RenderQueue::Get()->RemoveLight(lightId);
}

void Level::TurnONLight(ID lightId)
{
	if (not m_mapLights.contains(lightId)) return;
	if (m_mapLights[lightId].TurnOn) return;

	m_mapLights[lightId].TurnOn = true;
	RenderQueue::Get()->AddLight(m_mapLights[lightId].Light.get());
}

bool Level::IsLightOn(ID lightId)
{
	if (not m_mapLights.contains(lightId)) return false;
	return m_mapLights[lightId].TurnOn;
}

bool Level::IsAValidLight(ID lightId)
{
	return m_mapLights.contains(lightId);
}

const std::unordered_map<ID, ILightSource*>& Level::GetLightMapData()
{
	if (m_bDirtyLight) RebuildSafeLights();
	return m_safeMapLights;
}

ILightSource* Level::GetLightData(ID lightId)
{
	if (m_bDirtyLight) RebuildSafeLights();
	if (not m_safeMapLights.contains(lightId)) return nullptr;
	return m_safeMapLights[lightId];
}

void Level::LoadLights()
{
	for (auto& [id, light] : m_mapLights)
	{
		LoadLight(id);
	}
}

void Level::LoadLight(ID lightId)
{
	if (not m_mapLights.contains(lightId))   return;
	if (not m_mapLights[lightId].Light)		 return;
	if (not m_mapLights[lightId].TurnOn)	 return;
	if (m_eLevelState != LevelState::LOADED) return;

	RenderQueue::Get()->AddLight(m_mapLights[lightId].Light.get());
}

void Level::UnLoadLights()
{
	for (auto& [id, light] : m_mapLights)
	{
		UnLoadLight(id);
	}
}

void Level::UnLoadLight(ID lightId)
{
	RenderQueue::Get()->RemoveLight(lightId);
}

void Level::RebuildSafeLights()
{
	if (!m_bDirtyLight) return;
	m_bDirtyLight = false;

	m_safeMapLights.clear();

	for (auto& [id, data] : m_mapLights)
	{
		m_safeMapLights[id] = data.Light.get();
	}
}
