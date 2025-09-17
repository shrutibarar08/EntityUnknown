#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <functional>
#include <string>

#include "Category/GeneralCommands.h"
#include "Category/LevelCommands.h"
#include "Category/LightCommands.h"
#include "Category/MeshCommands.h"
#include "Category/SpriteCommands.h"

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
