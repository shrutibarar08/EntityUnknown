#pragma once
#include <cstdint>
#include <string>
#include <functional>

#include "LevelManager/LevelManager.h"
#include "Commands/Commands.h"
#include "Policies.h"
#include "SystemManager/Registry/RegistryTool.h"
#include "SystemManager/Registry/RegistryComponent.h"

// TODO: Make it template based instead according to level Editor policies
class LevelEditorContext
{
public:
    LevelEditorContext(
        LevelManager* pLevelManager,
        CommandStack* pCommandStack,
        EditorStorage* pStoragePolicy
    ):  m_pLevelManager(pLevelManager), m_pCommandStack(pCommandStack),
        m_pStorage(pStoragePolicy)
    {}
    ~LevelEditorContext() = default;

    LevelEditorContext(const LevelEditorContext&)            = delete;
    LevelEditorContext(LevelEditorContext&&)                 = delete;
    LevelEditorContext& operator=(const LevelEditorContext&) = delete;

    LevelManager*  GetLevelManager () const { return m_pLevelManager; }
    CommandStack*  GetCommandStack () const { return m_pCommandStack; }
    EditorStorage* GetStoragePolicy() const { return m_pStorage;      }

private:
    LevelManager*  m_pLevelManager  { nullptr };
    CommandStack*  m_pCommandStack  { nullptr };
    EditorStorage* m_pStorage       { nullptr };
};
