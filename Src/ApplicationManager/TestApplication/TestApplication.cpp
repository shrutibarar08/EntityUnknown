#include "TestApplication.h"

#include <random>

bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Ground = std::make_unique<ModelCube>();
	m_Ground->SetTexturePath("Texture/grass.tga");
	m_Ground->SetTextureMultiplier(13);
	m_Ground->GetRigidBody()->SetTranslationY(-2);
	m_Ground->SetScale(16, 1, 10);
	DirectX::XMVECTOR scale{ 16, 1, 10 };
	m_Ground->GetCubeCollider()->SetScale(scale);
	m_Ground->GetCubeCollider()->SetColliderState(ColliderState::Static);

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<float> cubeDistX(-10.0f, 10.0f);
	std::uniform_real_distribution<float> cubeDistY(-2.0f, 2.0f);
	std::uniform_real_distribution<float> cubeDistZ(5.0f, 30.0f);

	m_Left = std::make_unique<ModelCube>();
	m_Left->GetRigidBody()->SetTranslation(-20, 1, 2);
	DirectX::XMVECTOR scale_left{ 1, 1, 1 };
	m_Left->GetCubeCollider()->SetScale(scale_left);
	m_Left->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);

	m_Right = std::make_unique<ModelCube>();
	m_Right->GetRigidBody()->SetTranslation(3, 1, 2);
	m_Right->GetCubeCollider()->SetScale(scale_left);
	m_Right->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);

	Render3DQueue::AddModel(m_Left.get());
	Render3DQueue::AddModel(m_Right.get());

	std::uniform_real_distribution<float> distX(-20.0f, 5.0f);
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

		m_Clouds[i]->GetRigidBody()->SetTranslation(x, y, z);
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
	m_Background->GetRigidBody()->SetTranslation(580, 200);
	m_Background->SetPixelShaderPath(L"Shader/Bitmap/BitmapLightPS.hlsl");
	m_Background->SetVertexShaderPath(L"Shader/Bitmap/BitmapLightVS.hlsl");

	Render2DQueue::AddLight(m_Light.get());
	Render2DQueue::AddBackgroundSprite(m_Background.get());

	for (int i = 0; i < m_HeartSprite.size(); i++)
	{
		float Local_padding = topLeft + (padding * i);
		m_HeartSprite[i]->GetRigidBody()->SetTranslation(Local_padding, constantY);
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
	m_Light->SetSpecularPower(64.0f);											// Soft shine

	Render3DQueue::AddModel(m_Ground.get());
	Render3DQueue::AddLight(m_Light.get());

	BirdSprite_1 = std::make_unique<WorldSpaceSprite>();
	BirdSprite_2 = std::make_unique<WorldSpaceSprite>();
	BirdSprite_3 = std::make_unique<WorldSpaceSprite>();

	BirdSprite_1->SetScale(2.f, 2.f, 2.f);
	BirdSprite_1->GetRigidBody()->SetTranslation(-30, 6, 10.1);
	BirdSprite_1->SetTexturePath("Texture/background.tga");
	BirdSprite_1->SetPixelShaderPath(L"Shader/Bitmap/BitmapPlainPS.hlsl");
	BirdSprite_1->SetVertexShaderPath(L"Shader/Bitmap/BitmapPlainVS.hlsl");
	Render2DQueue::AddSpaceSprite(BirdSprite_1.get());

	BirdSprite_2->SetScale(2.f, 2.f, 2.f);
	BirdSprite_2->GetRigidBody()->SetTranslation(-28.3, 7.5, 10.2);
	BirdSprite_2->SetTexturePath("Texture/background.tga");
	BirdSprite_2->SetPixelShaderPath(L"Shader/Bitmap/BitmapPlainPS.hlsl");
	BirdSprite_2->SetVertexShaderPath(L"Shader/Bitmap/BitmapPlainVS.hlsl");
	Render2DQueue::AddSpaceSprite(BirdSprite_2.get());

	BirdSprite_3->SetScale(2.f, 2.f, 2.f);
	BirdSprite_3->GetRigidBody()->SetTranslation(-27, 5.0, 10.3);
	BirdSprite_3->SetTexturePath("Texture/background.tga");
	BirdSprite_3->SetPixelShaderPath(L"Shader/Bitmap/BitmapPlainPS.hlsl");
	BirdSprite_3->SetVertexShaderPath(L"Shader/Bitmap/BitmapPlainVS.hlsl");
	Render2DQueue::AddSpaceSprite(BirdSprite_3.get());

	m_BirdSpriteAnim_1 = std::make_unique<SpriteAnim>(BirdSprite_1.get());
	m_BirdSpriteAnim_1->SetMode(SpriteAnimMode::EqualTimePerFrame);
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/00_frame.tga");
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/01_frame.tga");
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/02_frame.tga");
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/03_frame.tga");
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/04_frame.tga");
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/05_frame.tga");
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/06_frame.tga");
	m_BirdSpriteAnim_1->AddFrame("Texture/bird/07_frame.tga");
	m_BirdSpriteAnim_1->Build(m_RenderSystem->GetDevice());
	m_BirdSpriteAnim_1->Play();

	m_BirdSpriteAnim_2 = std::make_unique<SpriteAnim>(BirdSprite_2.get());
	m_BirdSpriteAnim_2->SetMode(SpriteAnimMode::EqualTimePerFrame);
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/00_frame.tga");
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/01_frame.tga");
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/02_frame.tga");
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/03_frame.tga");
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/04_frame.tga");
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/05_frame.tga");
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/06_frame.tga");
	m_BirdSpriteAnim_2->AddFrame("Texture/bird/07_frame.tga");
	m_BirdSpriteAnim_2->Build(m_RenderSystem->GetDevice());
	m_BirdSpriteAnim_2->Play();

	m_BirdSpriteAnim_3 = std::make_unique<SpriteAnim>(BirdSprite_3.get());
	m_BirdSpriteAnim_3->SetMode(SpriteAnimMode::EqualTimePerFrame);
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/00_frame.tga");
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/01_frame.tga");
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/02_frame.tga");
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/03_frame.tga");
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/04_frame.tga");
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/05_frame.tga");
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/06_frame.tga");
	m_BirdSpriteAnim_3->AddFrame("Texture/bird/07_frame.tga");
	m_BirdSpriteAnim_3->Build(m_RenderSystem->GetDevice());
	m_BirdSpriteAnim_3->Play();

	m_Timer.Reset();
	return true;
}

bool TestApplication::Update()
{
	float deltaTime = m_Timer.Tick();
	m_BirdSpriteAnim_1->Update(deltaTime * 1.6f);
	m_BirdSpriteAnim_2->Update(deltaTime * 1.4f);
	m_BirdSpriteAnim_3->Update(deltaTime * 1.35f);

	BirdSprite_1->GetRigidBody()->AddTranslationX(deltaTime);
	BirdSprite_2->GetRigidBody()->AddTranslationX(deltaTime);
	BirdSprite_3->GetRigidBody()->AddTranslationX(deltaTime);
	m_Left->GetRigidBody()->AddTranslationX(deltaTime);

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
		cloud->GetRigidBody()->AddTranslationX(x);
	}
	return true;
}
