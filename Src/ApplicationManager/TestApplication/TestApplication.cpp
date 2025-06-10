#include "TestApplication.h"


bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Cube = std::make_unique<ModelCube>();
	m_Cube_2 = std::make_unique<ModelCube>();
	m_Light = std::make_unique<DirectionalLight>();

	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.50f, 0.25f, 1.0f);

	Render3DQueue::AddModel(m_Cube.get());
	Render3DQueue::AddModel(m_Cube_2.get());
	Render3DQueue::AddLight(m_Light.get());

	m_Cube_2->SetTranslationY(4);
	m_Cube_2->SetTranslationZ(5);
	m_Cube_2->SetScale(1.0f, 1.0f, 1.f);

	m_Cube->SetTranslationY(-2);
	m_Cube->SetScale(10, 2, 10);

	m_Timer.Reset();
	return true;
}

bool TestApplication::Update()
{
	float deltaTime = m_Timer.Tick();
	m_Cube_2->AddYaw(-2.0f * deltaTime);
	return true;
}
