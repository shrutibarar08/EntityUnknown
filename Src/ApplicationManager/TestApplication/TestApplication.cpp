#include "TestApplication.h"


bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Cube = std::make_unique<ModelCube>();
	m_Cube_2 = std::make_unique<ModelCube>();

	Render3DQueue::AddModel(m_Cube.get());
	Render3DQueue::AddModel(m_Cube_2.get());

	m_Cube_2->SetTranslationX(-4);

	m_Timer.Reset();
	return true;
}

bool TestApplication::Update()
{
	float deltaTime = m_Timer.Tick();

	m_Cube->AddRoll(2.0f * deltaTime);
	m_Cube->AddPitch(2.0f * deltaTime);
	m_Cube->AddYaw(2.0f * deltaTime);

	m_Cube_2->AddRoll(-2.0f * deltaTime);
	m_Cube_2->AddPitch(-2.0f * deltaTime);
	m_Cube_2->AddYaw(-2.0f * deltaTime);

	return true;
}
