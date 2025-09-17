#include "Commands.h"
#include <algorithm>

#include "Editor/Core/EditorContext.h"
#include "Editor/Core/LevelManager/LevelManager.h"

#include "Utils/Logger/Logger.h"


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
    std::string message = "CommandStack Executed: " + std::string(cmd->GetCommandName());
    m_done.emplace_back(std::move(cmd));
    m_undone.clear();
    if (m_done.size() > m_maxDepth)
        m_done.erase(m_done.begin());

    LOG_INFO(message);
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
