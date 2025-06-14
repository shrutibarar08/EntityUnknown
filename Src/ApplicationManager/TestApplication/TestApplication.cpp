#include "TestApplication.h"

#include <random>

bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Cube = std::make_unique<ModelCube>();
	m_Cube_2 = std::make_unique<ModelCube>();
	m_Cube_3 = std::make_unique<ModelCube>();

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<float> distX(-20.0f, -10.0f);
	std::uniform_real_distribution<float> distY(2.0f, 7.0f);
	std::uniform_real_distribution<float> distZ(1.0f, 15.0f);

	for (int i = 0; i < 15; i++)
	{
		m_Clouds.emplace_back(std::make_unique<WorldSpaceSprite>());
		m_Clouds[i]->SetVertexShaderPath(L"Shader/Sprite/SpaceSprite/VertexShader.hlsl");
		m_Clouds[i]->SetPixelShaderPath(L"Shader/Sprite/SpaceSprite/PixelShader.hlsl");

		float x = distX(rng);
		float y = distY(rng);
		float z = distZ(rng);

		m_Clouds[i]->SetTranslation(x, y, z);
		m_Clouds[i]->SetScale(3, 3, 3);
		m_Clouds[i]->SetTexturePath("Texture/cloud.tga");

		Render2DQueue::AddSpaceSprite(m_Clouds[i].get());
	}

	m_Light = std::make_unique<DirectionalLight>();

	m_Bitmaps.emplace_back(std::make_unique<IBitmap>());
	m_Bitmaps.emplace_back(std::make_unique<IBitmap>());
	m_Bitmaps.emplace_back(std::make_unique<IBitmap>());

	m_Background = std::make_unique<IBitmap>();
	m_Background->SetTexture("Texture/background.tga");
	m_Background->SetScaleXY(0.7f, 0.7f);
	m_Background->SetTranslation(580, 200);
	m_Background->SetPixelShader(L"Shader/Bitmap/BitmapLightPS.hlsl");
	m_Background->SetVertexShader(L"Shader/Bitmap/BitmapLightVS.hlsl");

	Render2DQueue::AddLight(m_Light.get());
	Render2DQueue::AddBackgroundSprite(m_Background.get());

	for (int i = 0; i < m_Bitmaps.size(); i++)
	{
		float Local_padding = topLeft + (padding * i);
		m_Bitmaps[i]->SetTranslation(Local_padding, constantY);
		m_Bitmaps[i]->SetTexture("Texture/health.tga");
		m_Bitmaps[i]->SetScaleXY(0.5f, 0.5f);
		m_Bitmaps[i]->SetPixelShader(L"Shader/Bitmap/BitmapPlainPS.hlsl");
		m_Bitmaps[i]->SetVertexShader(L"Shader/Bitmap/BitmapPlainVS.hlsl");
		Render2DQueue::AddScreenSprite(m_Bitmaps[i].get());
	}

	m_Light->SetDiffuseColor(1.0f, 0.95f, 0.85f, 1.0f);     // Warm soft white (sunlight)
	m_Light->SetDirection(-0.4f, -1.0f, 0.2f);              // Diagonal from above
	m_Light->SetAmbient(0.3f, 0.3f, 0.35f, 1.0f);           // Neutral ambient fill (sky bounce)
	m_Light->SetSpecularColor(1.0f, 0.95f, 0.85f, 1.0f);    // Sunny highlights
	m_Light->SetSpecularPower(32.0f);                      // Soft shine

	Render3DQueue::AddModel(m_Cube.get());
	Render3DQueue::AddModel(m_Cube_2.get());
	Render3DQueue::AddModel(m_Cube_3.get());
	Render3DQueue::AddLight(m_Light.get());

	m_Cube_2->SetTranslationY(2);
	m_Cube_2->SetTranslationX(1);
	m_Cube_2->SetScale(1.0f, 1.0f, 1.f);

	m_Cube->SetTranslationY(-2);
	m_Cube->SetScale(10, 2, 10);

	m_Cube_3->SetTranslationY(2);
	m_Cube_3->SetTranslationX(-2);
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
				Render2DQueue::RemoveScreenSprite(m_Bitmaps[index].get());
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
				Render2DQueue::AddScreenSprite(m_Bitmaps[index].get());
				LOG_INFO("Added Back on!");
			}

			if (index + 1 >= m_Bitmaps.size())
			{
				m_Removed = false;
			}
		}
	}

	// Move clouds to +X direction
	for (auto& cloud : m_Clouds)
	{
		float x = 0.15f * deltaTime;
		cloud->AddTranslationX(x);
	}
	return true;
}
