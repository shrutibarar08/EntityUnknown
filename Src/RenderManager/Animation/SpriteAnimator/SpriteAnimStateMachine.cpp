#include "SpriteAnimStateMachine.h"

#include <ranges>

#include "Imgui/imgui.h"

SpriteAnimStateMachine::SpriteAnimStateMachine(ISprite* sprite)
	: m_Sprite(sprite)
{
}

void SpriteAnimStateMachine::Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	for (auto& state: m_States | std::views::values)
	{
		state->Build(device, deviceContext);
	}
}

void SpriteAnimStateMachine::Update(float deltaTime)
{
	if (m_States.count(m_CurrentState))
		m_States[m_CurrentState]->Update(deltaTime);
}

void SpriteAnimStateMachine::AddState(const std::string& stateName)
{
    if (m_States.contains(stateName)) return;
    m_States[stateName] = std::make_unique<SpriteAnim>(m_Sprite);
}

void SpriteAnimStateMachine::AddState(const std::string& stateName, std::unique_ptr<SpriteAnim> anim)
{
	if (!anim) return;
	m_States[stateName] = std::move(anim);
}

void SpriteAnimStateMachine::SetInitialState(const std::string& name)
{
	m_CurrentState = name;
    if (m_States[name]) m_States[name]->Play();
}

void SpriteAnimStateMachine::TransitionTo(const std::string& name)
{
	if (name == m_CurrentState || m_States.find(name) == m_States.end())
		return;

	if (m_OnExitCallbacks.count(m_CurrentState)) m_OnExitCallbacks[m_CurrentState]();

	m_States[m_CurrentState]->Stop();
	m_PreviousState = m_CurrentState;
	m_CurrentState = name;
	m_States[name]->Play();

	if (m_OnEnterCallbacks.count(name)) m_OnEnterCallbacks[name]();
}

const std::string& SpriteAnimStateMachine::GetCurrentState() const { return m_CurrentState; }
const std::string& SpriteAnimStateMachine::GetPreviousState() const { return m_PreviousState; }
bool SpriteAnimStateMachine::IsInState(const std::string& name) const { return m_CurrentState == name; }

void SpriteAnimStateMachine::SetOnEnterCallback(const std::string& state, std::function<void()> callback)
{
	m_OnEnterCallbacks[state] = std::move(callback);
}

void SpriteAnimStateMachine::SetOnExitCallback(const std::string& state, std::function<void()> callback)
{
	m_OnExitCallbacks[state] = std::move(callback);
}

void SpriteAnimStateMachine::LoadFromSweetData(const SweetLoader& sweetData)
{
	m_States.clear();
	m_CurrentState.clear();
	m_PreviousState.clear();

	m_CurrentState = sweetData["CurrentState"].GetValue();
	m_PreviousState = sweetData["PreviousState"].GetValue();

	const auto& statesNode = sweetData["States"];
	for (const auto& [stateName, animData] : statesNode)
	{
		auto anim = std::make_unique<SpriteAnim>(m_Sprite);
		anim->LoadFromSweetData(animData);
		m_States[stateName] = std::move(anim);
	}
}

SweetLoader SpriteAnimStateMachine::GetSweetData()
{
	SweetLoader loader;
	loader.GetOrCreate("CurrentState") = m_CurrentState;
	loader.GetOrCreate("PreviousState") = m_PreviousState;

	auto& statesNode = loader.GetOrCreate("States");
	for (const auto& [stateName, anim] : m_States)
	{
		statesNode.GetOrCreate(stateName) = anim->GetSweetData();
	}

	return loader;
}

void SpriteAnimStateMachine::ControlUI()
{
    // === State Summary ===
    ImGui::Text("Current State: %s", m_CurrentState.c_str());
    ImGui::Text("Previous State: %s", m_PreviousState.c_str());

    // === State Selector ===
    if (ImGui::BeginCombo("Set State", m_CurrentState.c_str()))
    {
        for (auto& [name, anim] : m_States)
        {
            if (ImGui::Selectable(name.c_str(), name == m_CurrentState))
                TransitionTo(name);
        }
        ImGui::EndCombo();
    }

    // === Add New State ===
    static char newStateName[128] = {};
    ImGui::InputText("New State Name", newStateName, sizeof(newStateName));

    if (ImGui::Button("Add State"))
    {
        if (strlen(newStateName) > 0 && m_States.find(newStateName) == m_States.end())
        {
            auto anim = std::make_unique<SpriteAnim>(m_Sprite);
            AddState(newStateName, std::move(anim));
            newStateName[0] = '\0'; // Clear input
        }
    }

    ImGui::Separator();

    // === State Entries ===
    for (auto it = m_States.begin(); it != m_States.end(); )
    {
        const std::string& stateName = it->first;
        SpriteAnim* anim = it->second.get();

        ImGui::PushID(stateName.c_str());
        if (ImGui::TreeNode(stateName.c_str()))
        {
            // Set as initial
            if (ImGui::Button("Set As Initial"))
                SetInitialState(stateName);

            ImGui::SameLine();
            if (ImGui::Button("Play"))
                TransitionTo(stateName);

            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                it = m_States.erase(it); // Remove and move iterator
                ImGui::TreePop();
                ImGui::PopID();
                continue;
            }

            anim->ControlUI();

            ImGui::TreePop();
        }
        ImGui::PopID();
        ++it;
    }
}
