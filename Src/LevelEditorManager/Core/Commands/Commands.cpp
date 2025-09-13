#include "Commands.h"
#include <algorithm>

#include "LevelEditorManager/Core/EditorContext.h"
#include "LevelEditorManager/Core/LevelManager/LevelManager.h"

#include "RenderManager/Light/DefineLights.h"

#pragma region COMMAND_STACK
// ----------------- CommandStack -----------------

CommandStack::CommandStack() noexcept
    : done(m_done), undone(m_undone)
{
}

CommandStack::CommandStack(std::size_t maxDepth) noexcept
    : done(m_done), undone(m_undone), m_maxDepth(maxDepth)
{
}

CommandStack::CommandStack(CommandStack&& other) noexcept
    : done(m_done), undone(m_undone)
{
    m_done = std::move(other.m_done);
    m_undone = std::move(other.m_undone);
    m_maxDepth = other.m_maxDepth;
}

CommandStack& CommandStack::operator=(CommandStack&& other) noexcept
{
    if (this != &other)
    {
        m_done = std::move(other.m_done);
        m_undone = std::move(other.m_undone);
        m_maxDepth = other.m_maxDepth;
    }
    return *this;
}

void CommandStack::Execute(std::unique_ptr<ICommand> cmd, LevelEditorContext* ctx)
{
    if (!cmd) return;
    cmd->Do(ctx);
    m_done.emplace_back(std::move(cmd));
    m_undone.clear();
    if (m_done.size() > m_maxDepth)
        m_done.erase(m_done.begin()); // drop oldest to cap history
}

bool CommandStack::Undo(LevelEditorContext* ctx)
{
    if (m_done.empty()) return false;
    auto cmd = std::move(m_done.back());
    m_done.pop_back();
    cmd->Undo(ctx);
    m_undone.emplace_back(std::move(cmd));
    return true;
}

bool CommandStack::Redo(LevelEditorContext* ctx)
{
    if (m_undone.empty()) return false;
    auto cmd = std::move(m_undone.back());
    m_undone.pop_back();
    cmd->Do(ctx);
    m_done.emplace_back(std::move(cmd));
    return true;
}

void CommandStack::Clear() noexcept
{
    m_done.clear();
    m_undone.clear();
}

void CommandStack::ShrinkToFit()
{
    m_done.shrink_to_fit();
    m_undone.shrink_to_fit();
}

void CommandStack::SetMaxDepth(std::size_t depth) noexcept
{
    m_maxDepth = depth ? depth : 1;
    if (m_done.size() > m_maxDepth)
        m_done.erase(m_done.begin(), m_done.begin() + (m_done.size() - m_maxDepth));
}

std::size_t CommandStack::MaxDepth() const noexcept { return m_maxDepth; }
bool        CommandStack::CanUndo() const noexcept { return !m_done.empty(); }
bool        CommandStack::CanRedo() const noexcept { return !m_undone.empty(); }
std::size_t CommandStack::UndoDepth() const noexcept { return m_done.size(); }
std::size_t CommandStack::RedoDepth() const noexcept { return m_undone.size(); }

#pragma endregion

#pragma region COMMANDS

//=========================================================== Create Level Command Start
CmdCreateLevel::CmdCreateLevel(const std::string& opName)
    : m_szLevelName(opName)
{
}

const char* CmdCreateLevel::GetCommandName() const noexcept
{
    return "CreateLevel";
}

void CmdCreateLevel::Do(LevelEditorContext* ctx)
{
    auto* lm = ctx->GetLevelManager();
    if (not lm) return;

    if (auto* actice = lm->GetActiveLevel())
        m_szPrevActiveLevel = lm->GetActiveLevelName();

    if (lm->DoesLevelExists(m_szLevelName))
    {
        m_bExecutedCreateCommand = false;
        lm->SetActiveLevel(m_szLevelName);
        return;
    }

    lm->Create(m_szLevelName);
    lm->SetActiveLevel(m_szLevelName);
    m_bExecutedCreateCommand = true;
}

void CmdCreateLevel::Undo(LevelEditorContext* ctx)
{
    auto* lm = ctx->GetLevelManager();
    if (not lm) return;

    if (m_bExecutedCreateCommand)
    {
        lm->RemoveLevel(m_szLevelName);
        m_bExecutedCreateCommand = false;
    }

    if (!m_szPrevActiveLevel.empty() && lm->DoesLevelExists(m_szPrevActiveLevel))
    {
        lm->SetActiveLevel(m_szPrevActiveLevel);
    }
}
//=========================================================== Create Level Command End

//=========================================================== Create Level To Change Start
CmdSetActiveLevel::CmdSetActiveLevel(const std::string& szChangeToLevel)
    : m_szChangeTo(szChangeToLevel)
{}

const char* CmdSetActiveLevel::GetCommandName() const noexcept
{
    return "SwtichActiveLevel";
}

void CmdSetActiveLevel::Do(LevelEditorContext* context)
{
    auto* lm = context->GetLevelManager();
    if (not lm) return;

    m_szChangeFrom = lm->GetActiveLevelName();

    if (lm->DoesLevelExists(m_szChangeTo) && m_szChangeTo != m_szChangeFrom)
    {
        lm->SetActiveLevel(m_szChangeTo);
        m_bExecutedChangeCommand = m_szChangeFrom.empty() ? false : true;
    }
}

void CmdSetActiveLevel::Undo(LevelEditorContext* context)
{
    auto* lm = context->GetLevelManager();
    if (not lm) return;


    if (not m_bExecutedChangeCommand) return;
    m_bExecutedChangeCommand = false;

    if (!m_szChangeFrom.empty() && lm->DoesLevelExists(m_szChangeFrom))
    {
        lm->SetActiveLevel(m_szChangeFrom);
    }
}
//=========================================================== Create Level To Change End

//=========================================================== Light Command Start
CmdCreateLight::CmdCreateLight(std::unique_ptr<ILightSource> lightToAdd)
    : m_lightToBeAdded(std::move(lightToAdd))
{}

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
//=========================================================== Light Command End

//=========================================================== Light Turn On Or Off Command Start
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

    // Capture states only on first Do
    if (!m_bInitialized) 
    {
        m_bWasLightOn = level->IsLightOn(m_idLight);
        m_bWillBeOn = !m_bWasLightOn;
        m_bInitialized = true;
    }

    // Apply "next" state
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
//=========================================================== Light Turn On Or Off Command End

//=========================================================== Light Delete Operation Start
CmdDeleteLight::CmdDeleteLight(ID lightId)
    : m_idLight(lightId)
{}

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
//=========================================================== Light Delete Operation End
#pragma endregion

CmdRenameLight::CmdRenameLight(
    ILightSource* light,
    const std::string& changeFrom,
    const std::string& changeTo
)
    : m_pLight(light), m_szChangeFrom(changeFrom), m_szChangeTo(changeTo)
{}

const char* CmdRenameLight::GetCommandName() const noexcept
{
    return "LightRenameCommand";
}

void CmdRenameLight::Do(LevelEditorContext* context)
{
    if (!m_pLight) return;
    m_pLight->SetLightName(m_szChangeTo);
}

void CmdRenameLight::Undo(LevelEditorContext* context)
{
    if (!m_pLight) return;
    m_pLight->SetLightName(m_szChangeFrom);
}
