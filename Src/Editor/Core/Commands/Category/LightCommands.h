#pragma once
#include "Editor/Core/Commands/ICommand.h"
#include "RenderManager/Light/DefineLights.h"

#include <string>
#include <memory>

class CmdCreateLight final : public ICommand
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

class CmdToggleLightOnOrOff final : public ICommand
{
public:
    explicit CmdToggleLightOnOrOff(ID lightId);
    const char* GetCommandName() const noexcept override;
    void Do(LevelEditorContext* context) override;
    void Undo(LevelEditorContext* context) override;
private:
    ID m_idLight;
    std::string m_szLevel   {};
    bool m_bExecutedToggle  { false };
    bool m_bInitialized     { false };
    bool m_bWasLightOn      { false };
    bool m_bWillBeOn        { false };
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
