#pragma once
#include "RenderManager/IRender.h"
#include "RenderManager/ISystemRender.h"
#include "RenderManager/Model/Mesh/Mesh.h"
#include "SystemManager/ISystem.h"


class LevelEditor final: public ISystem, public ISystemRender
{
public:
	LevelEditor() = default;
	~LevelEditor() override = default;

	LevelEditor(const LevelEditor&) = delete;
	LevelEditor(LevelEditor&&) = delete;
	LevelEditor& operator=(const LevelEditor&) = delete;
	LevelEditor& operator=(LevelEditor&&) = delete;

	//~ For initializing Level Editor
	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnFrameUpdate(float deltaTime) override;
	bool OnFrameEnd() override;
	bool OnExit(SweetLoader& sweetLoader) override;
	std::string GetSystemName() override;

	//~ For ImGui or system level rendering
	void RenderBegin() override;
	void RenderExecute() override;
	void RenderEnd() override;

private:
	//~ UI
	void RenderMenuUI();

	void RenderObjectCubeCreationUI();
	void RenderObjectUpdateUI() const;

	//~ Obj Model
	void RenderObjectOBJCreationUI();

private:

	//~ Renders
	std::unordered_map<ID, std::unique_ptr<IRender>> m_Renders{};
	bool m_bDisplayRenderObjectUI{ false };
	bool m_bCreateCubeRenderObjectUI{ false };

	//~ Display and Render OBJ Models
	std::string m_RenderOBJPopUpName{ "Create OBJ Object" };
	bool m_bCreateOBJRenderObjectUI{ false };
	std::unique_ptr<Mesh> m_HolderMesh{ nullptr };
};
