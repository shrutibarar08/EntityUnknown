#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "SystemManager/ISystem.h"

class DependencyHandler
{
public:
	DependencyHandler() = default;
	~DependencyHandler() = default;

	DependencyHandler(const DependencyHandler&) = delete;
	DependencyHandler(DependencyHandler&&) = delete;
	DependencyHandler& operator=(const DependencyHandler&) = delete;
	DependencyHandler& operator=(DependencyHandler&&) = delete;

	void Register(ISystem* instance);
	void Clear();
	bool InitAll(const SweetLoader& sweetLoader);
	bool RunAll(float deltaTime) const;
	bool ShutdownAll(SweetLoader& sweetLoader);

	template<typename... Args>
	void AddDependency(ISystem* mainSystem, const Args*... dependencies);

private:
	std::vector<ISystem*> TopologicalSort();
	void DFS(const ISystem* node,
		std::unordered_set<ISystem*>& visited,
		std::unordered_set<ISystem*>& recursionStack,
		std::vector<ISystem*>& sorted);

private:
	std::unordered_set<ISystem*> m_Registry;
	std::unordered_map<ISystem*, std::vector<ISystem*>> m_Dependencies;
	std::vector<ISystem*> m_SystemNames;
	std::vector<ISystem*> m_InitOrder;
};

template<typename... Args>
void DependencyHandler::AddDependency(ISystem* mainSystem, const Args*... dependencies)
{
	(m_Dependencies[mainSystem].push_back(dependencies), ...);
}
