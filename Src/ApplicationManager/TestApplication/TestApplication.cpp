#include "TestApplication.h"


bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Cube = std::make_unique<ModelCube>();
	m_Cube_2 = std::make_unique<ModelCube>();
	m_Cube_3 = std::make_unique<ModelCube>();
	m_Light = std::make_unique<DirectionalLight>();
	m_Light_2 = std::make_unique<DirectionalLight>();

	m_Light->SetDiffuseColor(1.0f, 0.95f, 0.85f, 1.0f);     // Warm white (sunny glow)
	m_Light->SetDirection(-0.6f, -1.0f, -0.3f);							// Tilted downward, side-lit
	m_Light->SetAmbient(0.1f, 0.1f, 0.1f, 1.0f);            // Low neutral ambient
	m_Light->SetSpecularColor(1.0f, 0.95f, 0.85f, 1.0f);    // Same tone as diffuse
	m_Light->SetSpecularPower(32.f);											// Soft shine

	m_Light_2->SetDiffuseColor(0.1f, 0.2f, 0.5f, 1.0f);     // Cool blue fill
	m_Light_2->SetDirection(0.5f, -0.25f, 0.8f);							// Opposite angle, diagonal
	m_Light_2->SetAmbient(0.05f, 0.1f, 0.2f, 1.0f);         // Low cold ambient (color bounce)
	m_Light_2->SetSpecularColor(0.2f, 0.4f, 0.9f, 1.0f);    // Sharp blue specular
	m_Light_2->SetSpecularPower(128.f);											// Sharp rim highlights

	Render3DQueue::AddModel(m_Cube.get());
	Render3DQueue::AddModel(m_Cube_2.get());
	Render3DQueue::AddModel(m_Cube_3.get());
	Render3DQueue::AddLight(m_Light.get());
	Render3DQueue::AddLight(m_Light_2.get());

	m_Cube_2->SetTranslationY(4);
	m_Cube_2->SetTranslationZ(5);
	m_Cube_2->SetScale(1.0f, 1.0f, 1.f);

	m_Cube->SetTranslationY(-2);
	m_Cube->SetScale(10, 2, 10);

	m_Cube_3->SetTranslationY(4);
	m_Cube_3->SetTranslationX(-3);
	m_Cube_3->SetScale(1.0f, 1.0f, 1.f);

	m_Timer.Reset();
	return true;
}

bool TestApplication::Update()
{
	float deltaTime = m_Timer.Tick();
	m_Cube_2->AddYaw(deltaTime);
	m_Cube_3->AddYaw(-deltaTime);
	 
	WaitTime -= deltaTime;

	if (WaitTime <= 0.0f)
	{
		WaitTime += 2.0f;
		if (m_Removed)
		{
			LOG_INFO("GOING TO ADD LIGHT");
			//Render3DQueue::AddLight(m_Light.get());
			m_Removed = false;
		}else
		{
			LOG_INFO("GOING TO REMOVE LIGHT");
			m_Removed = true;
			//Render3DQueue::RemoveLight(m_Light.get());
		}
	}

	return true;
}
