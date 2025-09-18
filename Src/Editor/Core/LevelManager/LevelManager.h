#pragma once

#include "LevelModel.h"
#include "Utils/HelperFunctions.h"

/**
* Manages Multiple Levels
*/
class LevelManager
{
public:
    Level* CreateLevel(std::unique_ptr<Level> level, const std::string& levelName);
    Level* CreateLevel (const std::string& levelName);
    [[nodiscard]] Level* GetLevel    (const std::string& levelName);
    [[nodiscard]] Level* GetActiveLevel ();

    [[nodiscard]] const Level*       GetActiveLevel     () const;
    [[nodiscard]] const Level*       GetLevel        (const std::string& levelName) const;
    [[nodiscard]] const std::string& GetActiveLevelName () const noexcept;

    [[nodiscard]] std::vector<std::string>  Names() const;
    [[nodiscard]] std::size_t               TotalLevel()  const noexcept;

    [[nodiscard]] bool DoesLevelExists(const std::string& levelName) const;
    [[nodiscard]] bool RemoveLevel(const std::string& levelName);
    [[nodiscard]] std::unique_ptr<Level> RemoveAndGetLevel(const std::string& levelName);
    [[nodiscard]] bool IsEmpty () const noexcept;
    [[nodiscard]] bool IsAnActiveLevel(const std::string& levelName) const;
    [[nodiscard]] bool IsAnyLevelActive() const;

    void SetActiveLevel(const std::string& levelName);
    void ClearLevels();

    const std::vector<std::string>& GetLevelNames() const;

private:
    void RebuildNames();

private:
    std::unordered_map<std::string, std::unique_ptr<Level>> m_Levels;
    std::vector<std::string> m_pszNames{};
    std::string m_szActive{};
};
