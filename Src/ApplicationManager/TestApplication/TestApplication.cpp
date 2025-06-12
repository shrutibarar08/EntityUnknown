#include "TestApplication.h"


bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Cube = std::make_unique<ModelCube>();
	m_Cube_2 = std::make_unique<ModelCube>();
	m_Cube_3 = std::make_unique<ModelCube>();
	m_Light = std::make_unique<DirectionalLight>();
	m_Light_2 = std::make_unique<DirectionalLight>();

	m_Bitmaps.emplace_back(std::make_unique<IBitmap>());
	m_Bitmaps.emplace_back(std::make_unique<IBitmap>());
	m_Bitmaps.emplace_back(std::make_unique<IBitmap>());

	m_MainMenu = std::make_unique<IBitmap>();
	m_MainMenu->SetTexture("Texture/main-menu.tga");
	m_MainMenu->SetRenderPosition(0.0f, 1.0f);
	m_MainMenu->SetScale(4, 5, 1.f);
	Render2DQueue::AddBitmap(m_MainMenu.get());

	for (int i = 0; i < m_Bitmaps.size(); i++)
	{
		float Local_padding = topLeft + (padding * i);
		m_Bitmaps[i]->SetRenderPosition(Local_padding, constantY);
		m_Bitmaps[i]->SetTexture("Texture/health.tga");

		Render2DQueue::AddBitmap(m_Bitmaps[i].get());
	}

	m_Light->SetDiffuseColor(1.0f, 0.95f, 0.85f, 1.0f);     // Warm white (sunny glow)
	m_Light->SetDirection(1.f, -1.0f, 0.3f);							// Tilted downward, side-lit
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

	m_WaitTime -= deltaTime;

	static int index = static_cast<int>(m_Bitmaps.size()) - 1;

	if (m_WaitTime < 0.0f)
	{
		m_WaitTime += 3.f;

		if (!m_Removed)
		{
			if (index >= 0)
			{
				Render2DQueue::RemoveBitmap(m_Bitmaps[index].get());
				index--;

				if (index < 0)
					m_Removed = true;
			}
		}
		else // m_Removed == true
		{
			if (index + 1 < m_Bitmaps.size())
			{
				index++;
				Render2DQueue::AddBitmap(m_Bitmaps[index].get());
				LOG_INFO("Added Back on!");
			}

			if (index + 1 >= m_Bitmaps.size())
			{
				m_Removed = false;
			}
		}
	}

	return true;
}
