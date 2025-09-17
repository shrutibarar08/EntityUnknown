#include "LightCommands.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/Core/LevelManager/LevelManager.h"

#include "RenderManager/Light/DefineLights.h"

CmdCreateLight::CmdCreateLight(std::unique_ptr<ILightSource> lightToAdd)
    : m_lightToBeAdded(std::move(lightToAdd))
{
}

const char* CmdCreateLight::GetCommandName() const noexcept
{
    return "AddLightCommand";
}

void CmdCreateLight::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->IsAnyLevelActive()) return;

    if (m_szLevelName.empty())
        m_szLevelName = lm->GetActiveLevelName(); // only one time

    if (!m_lightToBeAdded) return;

    //~ In case if we doing redo and level is already deleted
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    m_idCreatedLight = m_lightToBeAdded->GetAssignedID();
    lm->GetLevel(m_szLevelName)->AddLight(std::move(m_lightToBeAdded));
    m_bExecutedAddLightCommand = true;
}

void CmdCreateLight::Undo(LevelEditorContext* context)
{
    if (!m_bExecutedAddLightCommand) return;
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;

    if (lm->DoesLevelExists(m_szLevelName))
    {
        m_lightToBeAdded = lm->GetLevel(m_szLevelName)->RemoveAndGetLight(m_idCreatedLight);
        m_bExecutedAddLightCommand = false;
    }
}

CmdToggleLightOnOrOff::CmdToggleLightOnOrOff(ID lightId)
    : m_idLight(lightId)
{
}

const char* CmdToggleLightOnOrOff::GetCommandName() const noexcept
{
    return "ToggleLightOnOrOffCommand";
}

void CmdToggleLightOnOrOff::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;

    // Capture target level once
    if (m_szLevel.empty())
    {
        if (!lm->IsAnyLevelActive()) return;
        m_szLevel = lm->GetActiveLevelName();
    }
    if (!lm->DoesLevelExists(m_szLevel)) return;

    auto* level = lm->GetLevel(m_szLevel);
    if (!level->IsAValidLight(m_idLight)) return;

    if (!m_bInitialized)
    {
        m_bWasLightOn = level->IsLightOn(m_idLight);
        m_bWillBeOn = !m_bWasLightOn;
        m_bInitialized = true;
    }

    if (m_bWillBeOn) level->TurnONLight(m_idLight);
    else             level->TurnOffLight(m_idLight);

    m_bExecutedToggle = true;
}

void CmdToggleLightOnOrOff::Undo(LevelEditorContext* context)
{
    if (!context) return;
    if (!m_bExecutedToggle) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->DoesLevelExists(m_szLevel)) return;

    auto* level = lm->GetLevel(m_szLevel);
    if (!level->IsAValidLight(m_idLight)) return;

    // Restore previous state
    if (m_bWasLightOn) level->TurnONLight(m_idLight);
    else               level->TurnOffLight(m_idLight);

    m_bExecutedToggle = false;
}

CmdDeleteLight::CmdDeleteLight(ID lightId)
    : m_idLight(lightId)
{
}

const char* CmdDeleteLight::GetCommandName() const noexcept
{
    return "DeleteLightCommand";
}

void CmdDeleteLight::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm)  return;

    if (m_szOperatedLevel.empty()) // save state
    {
        if (!lm->IsAnyLevelActive()) return;
        m_szOperatedLevel = lm->GetActiveLevelName();
    }

    if (!lm->DoesLevelExists(m_szOperatedLevel)) return;

    auto* lvl = lm->GetLevel(m_szOperatedLevel);
    if (!lvl) return;

    // Remove and stash
    m_deletedLight = lvl->RemoveAndGetLight(m_idLight);
    m_bExecutedDelete = (m_deletedLight != nullptr);
}

void CmdDeleteLight::Undo(LevelEditorContext* context)
{
    if (!m_bExecutedDelete) return;
    if (!context) return;

    auto* lm = context->GetLevelManager();
    if (!lm)  return;
    if (!lm->DoesLevelExists(m_szOperatedLevel)) return;

    auto* lvl = lm->GetLevel(m_szOperatedLevel);
    if (!lvl) return;

    if (m_deletedLight)
    {
        lvl->AddLight(std::move(m_deletedLight));
        m_bExecutedDelete = false;
    }
}

CmdRenameLight::CmdRenameLight(
    ILightSource* light,
    const std::string& changeTo
)
    : m_pLight(light), m_szChangeTo(changeTo)
{
}

const char* CmdRenameLight::GetCommandName() const noexcept
{
    return "LightRenameCommand";
}

void CmdRenameLight::Do(LevelEditorContext* context)
{
    if (!m_pLight) return;

    if (m_szChangeFrom.empty()) m_szChangeFrom = m_pLight->GetLightName();

    m_pLight->SetLightName(m_szChangeTo);
    m_bExecutedRename = true;
}

void CmdRenameLight::Undo(LevelEditorContext* context)
{
    if (!m_pLight) return;
    if (!m_bExecutedRename) return;

    m_pLight->SetLightName(m_szChangeFrom);
    m_bExecutedRename = false;
}