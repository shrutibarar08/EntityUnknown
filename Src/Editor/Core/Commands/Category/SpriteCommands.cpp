#include "SpriteCommands.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/Core/LevelManager/LevelManager.h"

CmdCreateBackgroundSprite::CmdCreateBackgroundSprite(const std::string& type)
    : m_szType(type)
{
}

const char* CmdCreateBackgroundSprite::GetCommandName() const noexcept
{
    return "CreateBackgroundSpriteCommand";
}

void CmdCreateBackgroundSprite::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;

    if (m_szLevelName.empty())
    {
        if (!lm->IsAnyLevelActive()) return;
        m_szLevelName = lm->GetActiveLevelName();
    }
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    if (!m_createdSprite)
    {
        m_createdSprite = RegistryMesh::CreateMesh(m_szType);
    }
    if (!m_createdSprite) return;

    m_bExecuted = true;
    m_createdSpriteId = m_createdSprite->GetAssignedID();
    lvl->AddBackgroundSprite(std::move(m_createdSprite));
    m_createdSprite = nullptr;
}

void CmdCreateBackgroundSprite::Undo(LevelEditorContext* context)
{
    if (!m_bExecuted) return;
    if (!context) return;

    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    m_bExecuted = false;
    m_createdSprite = lvl->RemoveAndGetBackgroundSprite(m_createdSpriteId);
}

CmdCreateFrontSprite::CmdCreateFrontSprite(const std::string& type)
    : m_szType(type)
{
}

const char* CmdCreateFrontSprite::GetCommandName() const noexcept
{
    return "CreateFrontSpriteCommand";
}

void CmdCreateFrontSprite::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;

    if (m_szLevelName.empty())
    {
        if (!lm->IsAnyLevelActive()) return;
        m_szLevelName = lm->GetActiveLevelName();
    }
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    if (!m_createdSprite)
    {
        m_createdSprite = RegistryMesh::CreateMesh(m_szType);
    }
    if (!m_createdSprite) return;

    m_bExecuted = true;
    m_createdSpriteId = m_createdSprite->GetAssignedID();
    lvl->AddFrontSprite(std::move(m_createdSprite));
    m_createdSprite = nullptr;
}

void CmdCreateFrontSprite::Undo(LevelEditorContext* context)
{
    if (!m_bExecuted) return;
    if (!context) return;

    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    m_bExecuted = false;
    m_createdSprite = lvl->RemoveAndGetFrontSprite(m_createdSpriteId);
}

CmdRemoveBackgroundSprite::CmdRemoveBackgroundSprite(ID spriteId)
    : m_targetId(spriteId)
{
}

const char* CmdRemoveBackgroundSprite::GetCommandName() const noexcept
{
    return "RemoveBackgroundSpriteCommand";
}

void CmdRemoveBackgroundSprite::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;

    if (m_szLevelName.empty())
    {
        if (!lm->IsAnyLevelActive()) return;
        m_szLevelName = lm->GetActiveLevelName();
    }
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    m_removedSprite = lvl->RemoveAndGetBackgroundSprite(m_targetId);
    if (!m_removedSprite) return;

    m_bExecuted = true;
}

void CmdRemoveBackgroundSprite::Undo(LevelEditorContext* context)
{
    if (!m_bExecuted) return;
    if (!context) return;

    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    lvl->AddBackgroundSprite(std::move(m_removedSprite));
    m_bExecuted = false;
}

CmdRemoveFrontSprite::CmdRemoveFrontSprite(ID spriteId)
    : m_targetId(spriteId)
{
}

const char* CmdRemoveFrontSprite::GetCommandName() const noexcept
{
    return "RemoveFrontSpriteCommand";
}

void CmdRemoveFrontSprite::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;

    if (m_szLevelName.empty())
    {
        if (!lm->IsAnyLevelActive()) return;
        m_szLevelName = lm->GetActiveLevelName();
    }
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    m_removedSprite = lvl->RemoveAndGetFrontSprite(m_targetId);
    if (!m_removedSprite) return;

    m_bExecuted = true;
}

void CmdRemoveFrontSprite::Undo(LevelEditorContext* context)
{
    if (!m_bExecuted) return;
    if (!context) return;

    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    lvl->AddFrontSprite(std::move(m_removedSprite));
    m_bExecuted = false;
}
