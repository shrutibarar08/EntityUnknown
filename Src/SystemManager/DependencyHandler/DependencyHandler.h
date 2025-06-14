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
	bool UpdateAllFrames(float deltaTime) const;
	bool EndAllFrames() const;
	bool ShutdownAll(SweetLoader& sweetLoader);

	template<typename... Args>
	void AddDependency(ISystem* mainSystem, Args*... dependencies);

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
void DependencyHandler::AddDependency(ISystem* mainSystem, Args*... dependencies)
{
	(m_Dependencies[mainSystem].emplace_back(dependencies), ...);
}
