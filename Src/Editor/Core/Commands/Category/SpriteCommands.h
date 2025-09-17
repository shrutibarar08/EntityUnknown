#pragma once
#include "Editor/Core/Commands/ICommand.h"
#include "RenderManager/DefineRenders.h"

#include <string>
#include <memory>

// ====================== BACKGROUND SPRITE ======================
class CmdCreateBackgroundSprite final : public ICommand
{
public:
    explicit CmdCreateBackgroundSprite(const std::string& type);
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    std::unique_ptr<IRender> m_createdSprite;
    ID m_createdSpriteId{ 0 };
    std::string m_szLevelName{};
    std::string m_szType;
    bool m_bExecuted{ false };
};

class CmdRemoveBackgroundSprite final : public ICommand
{
public:
    explicit CmdRemoveBackgroundSprite(ID spriteId);
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    std::unique_ptr<IRender> m_removedSprite;
    ID m_targetId{ 0 };
    std::string m_szLevelName{};
    bool m_bExecuted{ false };
};

// ====================== FRONT SPRITE ======================

class CmdCreateFrontSprite final : public ICommand
{
public:
    explicit CmdCreateFrontSprite(const std::string& type);
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    std::unique_ptr<IRender> m_createdSprite;
    ID m_createdSpriteId{ 0 };
    std::string m_szLevelName{};
    std::string m_szType;
    bool m_bExecuted{ false };
};

class CmdRemoveFrontSprite final : public ICommand
{
public:
    explicit CmdRemoveFrontSprite(ID spriteId);
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    std::unique_ptr<IRender> m_removedSprite;
    ID m_targetId{ 0 };
    std::string m_szLevelName{};
    bool m_bExecuted{ false };
};
