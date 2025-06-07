#include "DependencyHandler.h"
#include "Utils/Logger/Logger.h"

void DependencyHandler::Register(ISystem* instance)
{
	m_Registry.insert(instance);
}

void DependencyHandler::Clear()
{
	m_Registry.clear();
	m_Dependencies.clear();
	m_InitOrder.clear();
}

bool DependencyHandler::InitAll(const SweetLoader& sweetLoader)
{
    m_InitOrder = TopologicalSort();
    if (m_InitOrder.empty())
    {
        LOG_INFO("[DependencyHandler] Initialization failed: cyclic dependency detected.");
        return false;
    }

    int initCount = 1;
    for (ISystem* system : m_InitOrder)
    {
        if (!system->OnInit(sweetLoader))
        {
            LOG_INFO("[DependencyHandler] Failed to init: " + system->GetSystemName());
            return false;
        }
        else 
        {
           LOG_INFO("Initialized[System_" + std::to_string(initCount) + "]: " + system->GetSystemName());
           initCount++;
        }
    }

    LOG_INFO("All System Initialized!");
    return true;
}

bool DependencyHandler::RunAll(float deltaTime) const
{
    for (ISystem* system : m_InitOrder)
    {
        if (!system->OnTick(deltaTime))
        {
            LOG_INFO("[DependencyHandler] Failed to tick: " + system->GetSystemName());
        }
    }
    return true;
}

bool DependencyHandler::ShutdownAll(SweetLoader& sweetLoader)
{
    for (auto it = m_InitOrder.rbegin(); it != m_InitOrder.rend(); ++it)
    {
        if (!(*it)->OnExit(sweetLoader))
        {
            LOG_INFO("[DependencyHandler] Failed to shutdown: " + (*it)->GetSystemName());
        }
    }
    return true;
}

std::vector<ISystem*> DependencyHandler::TopologicalSort()
{
    std::unordered_set<ISystem*> visited;
    std::unordered_set<ISystem*> recursionStack;
    std::vector<ISystem*> sorted;

    for (ISystem* system : m_Registry)
    {
        if (visited.find(system) == visited.end())
        {
            DFS(system, visited, recursionStack, sorted);
            if (sorted.empty()) return {};
        }
    }

    std::reverse(sorted.begin(), sorted.end());
    return sorted;
}

void DependencyHandler::DFS(const ISystem* node,
	std::unordered_set<ISystem*>& visited,
	std::unordered_set<ISystem*>& recursionStack,
	std::vector<ISystem*>& sorted)
{
    if (recursionStack.contains(const_cast<ISystem*>(node)))
    {
        sorted.clear(); // cycle detected
        return;
    }

    if (visited.contains(const_cast<ISystem*>(node)))
        return;

    recursionStack.insert(const_cast<ISystem*>(node));
    visited.insert(const_cast<ISystem*>(node));

    auto it = m_Dependencies.find(const_cast<ISystem*>(node));
    if (it != m_Dependencies.end())
    {
        for (ISystem* dep : it->second)
        {
            DFS(dep, visited, recursionStack, sorted);
            if (sorted.empty()) return; // propagate cycle error
        }
    }

    recursionStack.erase(const_cast<ISystem*>(node));
    sorted.push_back(const_cast<ISystem*>(node));
}
