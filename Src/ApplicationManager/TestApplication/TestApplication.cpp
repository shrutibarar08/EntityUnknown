#include "TestApplication.h"

#include <random>

bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Ground = std::make_unique<ModelCube>();
	m_Ground->SetTexturePath("Texture/grass.tga");
	m_Ground->SetTextureMultiplier(13);
	m_Ground->SetTranslationY(-2);
	m_Ground->SetScale(16, 1, 10);

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<float> cubeDistX(-10.0f, 10.0f);
	std::uniform_real_distribution<float> cubeDistY(-2.0f, 2.0f);
	std::uniform_real_distribution<float> cubeDistZ(0.0f, 30.0f);

	for (int i = 0; i < 10; i++)
	{
		auto cube = std::make_unique<ModelCube>();

		float x = cubeDistX(rng);
		float y = cubeDistY(rng);
		float z = cubeDistZ(rng);

		cube->SetTranslation(x, y, z);
		Render3DQueue::AddModel(cube.get());
		m_Cubes.emplace_back(std::move(cube));
	}

	std::uniform_real_distribution<float> distX(-40.0f, -10.0f);
	std::uniform_real_distribution<float> distY(3.0f, 8.0f);
	std::uniform_real_distribution<float> distZ(1.0f, 15.0f);

	for (int i = 0; i < 40; i++)
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

	m_HeartSprite.emplace_back(std::make_unique<ScreenSprite>());
	m_HeartSprite.emplace_back(std::make_unique<ScreenSprite>());
	m_HeartSprite.emplace_back(std::make_unique<ScreenSprite>());

	m_Background = std::make_unique<BackgroundSprite>();
	m_Background->SetTexturePath("Texture/background.tga");
	m_Background->SetScaleXY(0.7f, 0.7f);
	m_Background->SetTranslation(580, 200);
	m_Background->SetPixelShaderPath(L"Shader/Bitmap/BitmapLightPS.hlsl");
	m_Background->SetVertexShaderPath(L"Shader/Bitmap/BitmapLightVS.hlsl");

	Render2DQueue::AddLight(m_Light.get());
	Render2DQueue::AddBackgroundSprite(m_Background.get());

	for (int i = 0; i < m_HeartSprite.size(); i++)
	{
		float Local_padding = topLeft + (padding * i);
		m_HeartSprite[i]->SetTranslation(Local_padding, constantY);
		m_HeartSprite[i]->SetTexturePath("Texture/health.tga");
		m_HeartSprite[i]->SetScaleXY(0.5f, 0.5f);
		m_HeartSprite[i]->SetPixelShaderPath(L"Shader/Bitmap/BitmapPlainPS.hlsl");
		m_HeartSprite[i]->SetVertexShaderPath(L"Shader/Bitmap/BitmapPlainVS.hlsl");
		Render2DQueue::AddScreenSprite(m_HeartSprite[i].get());
	}

	m_Light->SetDiffuseColor(1.0f, 0.95f, 0.85f, 1.0f);     // Warm soft white (sunlight)
	m_Light->SetDirection(0.40f, -1.0f, 1.0f);
	m_Light->SetAmbient(0.3f, 0.3f, 0.35f, 1.0f);           // Neutral ambient fill (sky bounce)
	m_Light->SetSpecularColor(1.0f, 0.95f, 0.85f, 1.0f);    // Sunny highlights
	m_Light->SetSpecularPower(64.0f);                      // Soft shine

	Render3DQueue::AddModel(m_Ground.get());
	Render3DQueue::AddLight(m_Light.get());

	m_AnimationSpriteHolder = std::make_unique<WorldSpaceSprite>();
	m_AnimationSpriteHolder->SetScale(3.f, 3.f, 3.f);
	m_AnimationSpriteHolder->SetTranslation(0, 0, 0);
	m_AnimationSpriteHolder->SetTexturePath("Texture/background.tga");
	m_AnimationSpriteHolder->SetPixelShaderPath(L"Shader/Bitmap/BitmapPlainPS.hlsl");
	m_AnimationSpriteHolder->SetVertexShaderPath(L"Shader/Bitmap/BitmapPlainVS.hlsl");
	Render2DQueue::AddSpaceSprite(m_AnimationSpriteHolder.get());

	m_SpriteAnim = std::make_unique<SpriteAnim>(m_AnimationSpriteHolder.get());
	m_SpriteAnim->SetMode(SpriteAnimMode::EqualTimePerFrame);
	m_SpriteAnim->AddFrame("Texture/bird/00_frame.tga");
	m_SpriteAnim->AddFrame("Texture/bird/01_frame.tga");
	m_SpriteAnim->AddFrame("Texture/bird/02_frame.tga");
	m_SpriteAnim->AddFrame("Texture/bird/03_frame.tga");
	m_SpriteAnim->AddFrame("Texture/bird/04_frame.tga");
	m_SpriteAnim->AddFrame("Texture/bird/05_frame.tga");
	m_SpriteAnim->AddFrame("Texture/bird/06_frame.tga");
	m_SpriteAnim->AddFrame("Texture/bird/07_frame.tga");
	m_SpriteAnim->Build(m_RenderSystem->GetDevice());
	m_SpriteAnim->Play();
	m_Timer.Reset();
	return true;
}

bool TestApplication::Update()
{
	float deltaTime = m_Timer.Tick();
	m_SpriteAnim->Update(deltaTime);

	for (int i = 0; i < 10; i++)
	{
		if (i & 1) m_Cubes[i]->AddYaw(deltaTime);
		else m_Cubes[i]->AddYaw(-deltaTime);
	}

	m_WaitTime -= deltaTime;

	static int index = static_cast<int>(m_HeartSprite.size()) - 1;

	if (m_WaitTime < 0.0f)
	{
		m_WaitTime += 3.f;

		if (!m_Removed)
		{
			if (index >= 0)
			{
				Render2DQueue::RemoveScreenSprite(m_HeartSprite[index].get());
				index--;

				if (index < 0)
					m_Removed = true;
			}
		}
		else // m_Removed == true
		{
			if (index + 1 < m_HeartSprite.size())
			{
				index++;
				Render2DQueue::AddScreenSprite(m_HeartSprite[index].get());
			}

			if (index + 1 >= m_HeartSprite.size())
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
