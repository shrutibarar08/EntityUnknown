#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <functional>
#include <string>

#include "ICommand.h"

#include "SystemManager/PrimaryID.h"

// Forward declaration to avoid heavy includes here
class LevelEditorContext;
class ILightSource;

class CommandStack
{
public:
    CommandStack() noexcept;
    explicit CommandStack(std::size_t maxDepth) noexcept;

    CommandStack(CommandStack&& other) noexcept;
    CommandStack& operator=(CommandStack&& other) noexcept;

    CommandStack(const CommandStack&)            = delete;
    CommandStack& operator=(const CommandStack&) = delete;

    ~CommandStack() = default;

    // --- core operations ---
    void Execute(std::unique_ptr<ICommand> cmd, LevelEditorContext* ctx);
    bool Undo(LevelEditorContext* ctx);
    bool Redo(LevelEditorContext* ctx);

    void Clear() noexcept;
    void ShrinkToFit();

    // --- config ---
    void        SetMaxDepth(std::size_t depth) noexcept;
    std::size_t MaxDepth() const noexcept;

    // --- queries ---
    bool        CanUndo() const noexcept;
    bool        CanRedo() const noexcept;
    std::size_t UndoDepth() const noexcept;
    std::size_t RedoDepth() const noexcept;

    const std::vector<std::unique_ptr<ICommand>>& done;
    const std::vector<std::unique_ptr<ICommand>>& undone;

private:
    // internal storage
    std::vector<std::unique_ptr<ICommand>> m_done;
    std::vector<std::unique_ptr<ICommand>> m_undone;
    std::size_t m_maxDepth{ 256 };
};

class CmdCreateLevel final: public ICommand
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

class CmdCreateLight final: public ICommand
{
public:
    explicit CmdCreateLight(std::unique_ptr<ILightSource> lightToAdd);
    const char* GetCommandName() const noexcept override;
    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;
private:
    ID                            m_idCreatedLight{};
    std::unique_ptr<ILightSource> m_lightToBeAdded;
    bool                          m_bExecutedAddLightCommand{ false };
    std::string                   m_szLevelName{};
};

class CmdToggleLightOnOrOff final: public ICommand
{
public:
    explicit CmdToggleLightOnOrOff(ID lightId);
    const char* GetCommandName() const noexcept override;
    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;
private:
    ID m_idLight;
    std::string m_szLevel{};
    bool m_bExecutedToggle{ false };
    bool m_bInitialized{ false };
    bool m_bWasLightOn{ false };
    bool m_bWillBeOn{ false };
};

class CmdDeleteLight final : public ICommand
{
public:
    explicit CmdDeleteLight(ID lightId);
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    ID m_idLight;
    std::string m_szOperatedLevel{};
    bool m_bExecutedDelete{ false };
    std::unique_ptr<ILightSource> m_deletedLight;
};

class CmdRenameLight final : public ICommand
{
public:
    explicit CmdRenameLight(
        ILightSource* light,
        const std::string& changeTo
    );
    const char* GetCommandName() const noexcept override;

    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;

private:
    ILightSource* m_pLight;
    std::string m_szChangeTo{};
    std::string m_szChangeFrom{};
    bool m_bExecutedRename{ false };
};
