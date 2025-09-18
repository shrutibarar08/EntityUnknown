#include "LevelManager.h"
#include "Utils/Logger/Logger.h"
#include <assert.h>

Level* LevelManager::CreateLevel(std::unique_ptr<Level> level, const std::string& levelName)
{
    m_Levels[levelName] = std::move(level);
    m_Levels[levelName]->SetName(levelName);
    RebuildNames();
    return m_Levels[levelName].get();
}

Level* LevelManager::CreateLevel(const std::string& levelName)
{
    if (m_Levels.contains(levelName)) return m_Levels[levelName].get();

    auto [ins, _] = m_Levels.emplace(levelName, std::make_unique<Level>());
    ins->second->SetName(ins->first);
    m_pszNames.emplace_back(levelName);
    return ins->second.get();
}

Level* LevelManager::GetLevel(const std::string& levelName)
{
    auto it = m_Levels.find(levelName);
    return (it == m_Levels.end()) ? nullptr : it->second.get();
}

Level* LevelManager::GetActiveLevel()
{
    if (m_szActive.empty()) return nullptr;
    return m_Levels[m_szActive].get();
}

const Level* LevelManager::GetActiveLevel() const
{
    if (m_szActive.empty()) return nullptr;
    auto it = m_Levels.find(m_szActive);
    return (it == m_Levels.end()) ? nullptr : it->second.get();
}

const Level* LevelManager::GetLevel(const std::string& levelName) const
{
    auto it = m_Levels.find(levelName);
    return (it == m_Levels.end()) ? nullptr : it->second.get();
}

const std::string& LevelManager::GetActiveLevelName() const noexcept
{
    return m_szActive;
}

std::size_t LevelManager::TotalLevel() const noexcept
{
    return m_Levels.size();
}

bool LevelManager::DoesLevelExists(const std::string& levelName) const
{
    return m_Levels.contains(levelName);
}

bool LevelManager::RemoveLevel(const std::string& levelName)
{
    if (m_szActive == levelName)
    {
        m_Levels[levelName]->UnHook();
        m_szActive.clear();
    } 

    if (m_Levels.erase(levelName) > 0)
    {
        RebuildNames();
        return true;
    }

    return false;
}

std::unique_ptr<Level> LevelManager::RemoveAndGetLevel(const std::string& levelName)
{
    if (!DoesLevelExists(levelName)) return nullptr;

    if (m_szActive == levelName)
    {
        m_Levels[levelName]->UnHook();
        m_szActive.clear();
    } 

    auto level = std::move(m_Levels[levelName]);
    m_Levels.erase(levelName);
    RebuildNames();

    return level;
}

bool LevelManager::IsEmpty() const noexcept
{
    return m_Levels.empty();
}

bool LevelManager::IsAnActiveLevel(const std::string& levelName) const
{
    return m_szActive == levelName;
}

bool LevelManager::IsAnyLevelActive() const
{
    return not m_szActive.empty() && DoesLevelExists(m_szActive);
}

void LevelManager::SetActiveLevel(const std::string& levelName)
{
    if (m_Levels.find(levelName) == m_Levels.end())
    {
        assert(false && "SetActive: level name does not exist");
        return;
    }
    if (m_szActive != levelName && DoesLevelExists(m_szActive))
    {
        m_Levels[m_szActive]->UnHook();
    }
    m_szActive = levelName;
    m_Levels[m_szActive]->Hook();

    LOG_INFO("Changed To: " + levelName);
}

void LevelManager::ClearLevels()
{
    if (DoesLevelExists(m_szActive))
    {
        m_Levels[m_szActive]->UnHook();
    }
    m_Levels.clear(); m_szActive.clear(); m_pszNames.clear();
}

const std::vector<std::string>& LevelManager::GetLevelNames() const
{
    return m_pszNames;
}

void LevelManager::RebuildNames()
{
    m_pszNames.clear();
    for (const auto& level : m_Levels)
    {
        m_pszNames.emplace_back(level.second->GetName());
    }
}
