#pragma once
#include <d3d11.h>

#include "RenderManager/Model/ModelLoader/ModelLoader.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "RenderManager/Model/IModel.h"
#include "RenderManager/Model/Cube/ModelCube.h"


class Mesh final : public IModel
{
public:
	Mesh()				= default;
	~Mesh() override	= default;

	Mesh(const Mesh&)				= delete;
	Mesh(Mesh&&)					= delete;
	Mesh& operator=(const Mesh&)	= delete;
	Mesh& operator=(Mesh&&)			= delete;

	void SetMeshPath(const std::string& path);
	bool IsInitialized() const override;

	void RenderControlUI() override;
	void SetSweetData(const SweetLoader& sweetData) override;
	SweetLoader GetSweetData() const override;

protected:
	bool BuildChild(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	bool RenderChild(ID3D11DeviceContext* deviceContext) override;
	void BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	void RenderGeometry(ID3D11DeviceContext* deviceContext) override;

private:
	bool m_Initialized{ false };
	std::string m_MeshPath{ "None" };
	std::unique_ptr<StaticVBInstance<MeshBuffer>> m_MeshBuffer{ nullptr };

	inline static std::unique_ptr<ModelCube> m_DebugCube{ nullptr };
};
