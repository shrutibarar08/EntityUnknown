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
        SweetLoaderStoragePolicy* pStoragePolicy,
        ImGuiPolicy* pUiPolicy
    ):  m_pLevelManager(pLevelManager), m_pCommandStack(pCommandStack),
        m_pStorage(pStoragePolicy),     m_pUserIneraction(pUiPolicy)
    {}
    ~LevelEditorContext() = default;

    LevelEditorContext(const LevelEditorContext&)            = delete;
    LevelEditorContext(LevelEditorContext&&)                 = delete;
    LevelEditorContext& operator=(const LevelEditorContext&) = delete;

    LevelManager*             GetLevelManager () const { return m_pLevelManager;   }
    CommandStack*             GetCommandStack () const { return m_pCommandStack;   }
    SweetLoaderStoragePolicy* GetStoragePolicy() const { return m_pStorage;        }
    ImGuiPolicy*              GetUIpolicy     () const { return m_pUserIneraction; }

private:
    LevelManager*             m_pLevelManager  { nullptr };
    CommandStack*             m_pCommandStack  { nullptr };
    SweetLoaderStoragePolicy* m_pStorage       { nullptr };
    ImGuiPolicy*              m_pUserIneraction{ nullptr };
};
