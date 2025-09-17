#pragma once
#include "Editor/Core/Commands/ICommand.h"
#include "RenderManager/DefineRenders.h"

#include <string>
#include <memory>

// ====================== MESH ======================
class CmdCreateMesh final : public ICommand
{
public:
    explicit CmdCreateMesh(const std::string& type);
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    std::unique_ptr<IRender> m_createdMesh;
    ID m_createdMeshId{ 0 };
    std::string m_szLevelName{};
    std::string m_szType;
    bool m_bExecuted{ false };
};


class CmdRemoveMesh final : public ICommand
{
public:
    explicit CmdRemoveMesh(ID meshId);
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    std::unique_ptr<IRender> m_removedMesh;
    ID m_targetId{ 0 };
    std::string m_szLevelName{};
    bool m_bExecuted{ false };
};
