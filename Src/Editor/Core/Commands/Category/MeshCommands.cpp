#include "MeshCommands.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/Core/LevelManager/LevelManager.h"

CmdCreateMesh::CmdCreateMesh(const std::string& type)
    : m_szType(type) {
}

const char* CmdCreateMesh::GetCommandName() const noexcept
{
    return "CreateMeshCommand";
}

void CmdCreateMesh::Do(LevelEditorContext* context)
{
    if (!context) return;
    auto* lm = context->GetLevelManager();
    if (!lm) return;

    if (m_szLevelName.empty()) {
        if (!lm->IsAnyLevelActive()) return;
        m_szLevelName = lm->GetActiveLevelName();
    }
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    if (!m_createdMesh) {
        m_createdMesh = RegistryMesh::CreateMesh(m_szType);
    }
    if (!m_createdMesh) return;

    m_bExecuted = true;
    m_createdMeshId = m_createdMesh->GetAssignedID();
    lvl->AddMesh(std::move(m_createdMesh));
    m_createdMesh = nullptr;
}

void CmdCreateMesh::Undo(LevelEditorContext* context)
{
    if (!m_bExecuted) return;
    if (!context) return;

    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    m_bExecuted = false;
    m_createdMesh = lvl->RemoveAndGetMesh(m_createdMeshId);
}

CmdRemoveMesh::CmdRemoveMesh(ID meshId)
    : m_targetId(meshId)
{
}

const char* CmdRemoveMesh::GetCommandName() const noexcept
{
    return "RemoveMeshCommand";
}

void CmdRemoveMesh::Do(LevelEditorContext* context)
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

    m_removedMesh = lvl->RemoveAndGetMesh(m_targetId);
    if (!m_removedMesh) return;

    m_bExecuted = true;
}

void CmdRemoveMesh::Undo(LevelEditorContext* context)
{
    if (!m_bExecuted) return;
    if (!context) return;

    auto* lm = context->GetLevelManager();
    if (!lm) return;
    if (!lm->DoesLevelExists(m_szLevelName)) return;

    auto* lvl = lm->GetLevel(m_szLevelName);
    if (!lvl) return;

    lvl->AddMesh(std::move(m_removedMesh));
    m_bExecuted = false;
}
