#include "LevelCommands.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/Core/LevelManager/LevelManager.h"

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

    lm->CreateLevel(m_szLevelName);
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

CmdSetActiveLevel::CmdSetActiveLevel(const std::string& szChangeToLevel)
    : m_szChangeTo(szChangeToLevel)
{
}

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

CmdDeleteLevel::CmdDeleteLevel(const std::string& szLevelName)
    : m_szDeletedLevelName(szLevelName)
{}

const char* CmdDeleteLevel::GetCommandName() const noexcept
{
    return "LevelDeleteCommand";
}

void CmdDeleteLevel::Do(LevelEditorContext* context)
{
    if (!context) return;
    if (!context->GetLevelManager()) return;
    if (m_bExecuted) return;
    if (!context->GetLevelManager()->DoesLevelExists(m_szDeletedLevelName)) return;

    m_cachedLevel = std::move(context->GetLevelManager()->RemoveAndGetLevel(m_szDeletedLevelName));
    m_bExecuted = true;
}

void CmdDeleteLevel::Undo(LevelEditorContext* context)
{
    if (!context) return;
    if (!context->GetLevelManager()) return;
    if (!m_bExecuted) return;

    if (!m_cachedLevel) return;
    context->GetLevelManager()->CreateLevel(std::move(m_cachedLevel), m_szDeletedLevelName);
    m_bExecuted = false;
}
