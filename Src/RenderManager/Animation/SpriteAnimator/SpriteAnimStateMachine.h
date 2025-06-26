#pragma once
#include <string>
#include <unordered_map>
#include <functional>

#include "SpriteAnim.h"

class SpriteAnimStateMachine
{
public:
    SpriteAnimStateMachine(ISprite* sprite);
    ~SpriteAnimStateMachine() = default;

    void Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    void Update(float deltaTime);

    // === State Management ===
    void AddState(const std::string& stateName);
    void AddState(const std::string& stateName, std::unique_ptr<SpriteAnim> anim);
    void SetInitialState(const std::string& name);
    void TransitionTo(const std::string& name);

    const std::string& GetCurrentState() const;
    const std::string& GetPreviousState() const;
    bool IsInState(const std::string& name) const;

    // === Optional hooks ===
    void SetOnEnterCallback(const std::string& state, std::function<void()> callback);
    void SetOnExitCallback(const std::string& state, std::function<void()> callback);

    void LoadFromSweetData(const SweetLoader& sweetData);
    SweetLoader GetSweetData();

    void ControlUI();

private:
    ISprite* m_Sprite{ nullptr };

    std::unordered_map<std::string, std::unique_ptr<SpriteAnim>> m_States;
    std::unordered_map<std::string, std::function<void()>> m_OnEnterCallbacks;
    std::unordered_map<std::string, std::function<void()>> m_OnExitCallbacks;

    std::string m_CurrentState;
    std::string m_PreviousState;
};
