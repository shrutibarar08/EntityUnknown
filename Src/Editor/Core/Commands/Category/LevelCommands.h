#pragma once
#include "Editor/Core/Commands/ICommand.h"

#include <string>

class CmdCreateLevel final : public ICommand
{
public:
    explicit CmdCreateLevel(const std::string& opName);

    const char* GetCommandName() const noexcept override;
    void Do(LevelEditorContext* ctx) override;
    void Undo(LevelEditorContext* ctx) override;

private:
    std::string m_szLevelName;
    std::string m_szPrevActiveLevel;
    bool m_bExecutedCreateCommand{ false };
};

class CmdSetActiveLevel final : public ICommand
{
public:
    explicit CmdSetActiveLevel(const std::string& szChangeToLevel);

    const char* GetCommandName() const noexcept override;
    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    std::string m_szChangeTo{};
    std::string m_szChangeFrom{};
    bool m_bExecutedChangeCommand{ false };
};

class CmdDeleteLevel final: public ICommand{};
