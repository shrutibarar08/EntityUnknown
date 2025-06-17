#include "TestApplication.h"

#include <random>

#include "External/Imgui/imgui.h"

bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Ground = std::make_unique<ModelCube>();
	m_Ground->GetRigidBody()->SetTranslation(0, -2, 1);
	DirectX::XMVECTOR scale_ground{ 20, 1, 20 };
	m_Ground->GetCubeCollider()->SetScale(scale_ground);
	m_Ground->SetScale(scale_ground);
	m_Ground->SetTexturePath("Texture/sample.tga");
    m_Ground->SetOptionalTexturePath("Texture/test.tga");
	m_Ground->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);
	Render3DQueue::AddModel(m_Ground.get());

	m_Right = std::make_unique<ModelCube>();
	m_Right->GetRigidBody()->SetTranslation(3, 1, 2);
	DirectX::XMVECTOR scale_right{ 2, 1, 1 };
	m_Right->GetCubeCollider()->SetScale(scale_right);
	m_Right->SetScale(scale_right);
	m_Right->GetCubeCollider()->SetColliderState(ColliderState::Trigger);

    m_Left = std::make_unique<ModelCube>();

    m_GhostSprite = std::make_unique<ScreenSprite>();
    m_GhostSprite->SetTexturePath("Texture/ghost.tga");
    m_GhostSprite->SetEdgePercents(1.0f, 1.0f, 1.0f, 1.0f);

    TRIGGER_COLLISION_INFO info{};
    info.m_OnTriggerEnterCallbackFn = [&]()
    {
        Render2DQueue::AddScreenSprite(m_GhostSprite.get());
    };
    info.m_OnTriggerExitCallbackFn = [&]()
    {
        Render2DQueue::RemoveScreenSprite(m_GhostSprite.get());
    };
    info.TargetCollider = m_Left->GetCubeCollider();

    m_Right->GetCubeCollider()->SetTriggerTarget(info);

	Render3DQueue::AddModel(m_Right.get());

	m_Left->GetRigidBody()->SetTranslation(-2, 1, 2);
	DirectX::XMVECTOR scale_left{ 1, 1, 1 };
	m_Left->GetCubeCollider()->SetScale(scale_left);
	m_Left->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);
	m_Left->SetTexturePath("Texture/test.tga");

	Render3DQueue::AddModel(m_Left.get());

	m_DirectionalLight = std::make_unique<DirectionalLight>();
	m_DirectionalLight->SetDiffuseColor(0.50f, 0.45f, 0.45f, 1.0f);
	m_DirectionalLight->SetDirection(0.40f, -1.0f, 1.0f);
	m_DirectionalLight->SetAmbient(0.3f, 0.3f, 0.35f, 1.0f);
	m_DirectionalLight->SetSpecularColor(1.0f, 0.95f, 0.85f, 1.0f);
	m_DirectionalLight->SetSpecularPower(64.0f);
	Render3DQueue::AddLight(m_DirectionalLight.get());

	m_SpotLight = std::make_unique<SpotLight>();
	m_SpotLight->SetAmbient(0.1f, 0.1f, 0.1f, 1.0f);
	m_SpotLight->SetDiffuseColor(1.0f, 0.9f, 0.7f, 1.0f);
	m_SpotLight->SetSpecularColor(1.0f, 0.95f, 0.9f, 1.0f);
	m_SpotLight->SetSpecularPower(64.0f);
	m_SpotLight->SetPosition(0.0f, 3.0f, 4.0f);
	m_SpotLight->SetDirection(0.400f, -1.0f, 1.f);
	m_SpotLight->SetRange(100.0f);
	m_SpotLight->SetSpotAngleDegrees(30.0f);
	Render3DQueue::AddLight(m_SpotLight.get());

    m_PointLight = std::make_unique<PointLight>();
    m_PointLight->SetAmbient(0.1f, 0.1f, 0.1f, 1.0f);
    m_PointLight->SetDiffuseColor(1.0f, 0.8f, 0.6f, 1.0f);
    m_PointLight->SetSpecularColor(1.0f, 0.95f, 0.9f, 1.0f);
    m_PointLight->SetSpecularPower(64.0f);
    m_PointLight->SetPosition(0.0f, 3.0f, 3.0f);
    m_PointLight->SetRange(8.0f);
    Render3DQueue::AddLight(m_PointLight.get());

    // 2D Sprite
    m_Bird = std::make_unique<WorldSpaceSprite>();
    m_Bird->SetTexturePath("Texture/bird/00_frame.tga");
    m_Bird->AddScale(2.0f, 2.0f);
    m_Bird->GetRigidBody()->AddTranslation(0, 4, 2);

    Render2DQueue::AddSpaceSprite(m_Bird.get());
    Render2DQueue::AddLight(m_PointLight.get());
    Render2DQueue::AddLight(m_SpotLight.get());
    Render2DQueue::AddLight(m_DirectionalLight.get());

    m_Background = std::make_unique<BackgroundSprite>();
    m_Background->GetRigidBody()->SetTranslation(0, 0, 0);
    m_Background->SetEdgePercents(1.f, 1.f, 1.f, 1.f);
    m_Background->SetTexturePath("Texture/test.tga");
    m_Background->GetRigidBody()->SetYaw(1.57f);
    Render2DQueue::AddBackgroundSprite(m_Background.get());

	m_Timer.Reset();
	return true;
}

void TestApplication::Update()
{
    float deltaTime = m_Timer.Tick();
    m_Left->GetRigidBody()->AddYaw(deltaTime);
}

void TestApplication::RenderBegin()
{
    SpotLightControl();
    DirectionalLightControl();
    PointLightControl();
    BackgroundControl();
    LeftCubeControl();
    GhostControl();
}

void TestApplication::RenderExecute()
{
}

void TestApplication::RenderEnd()
{
}

void TestApplication::SpotLightControl()
{
    ImGui::Begin("Spot Light Editor");

    if (m_SpotLight)
    {
        // Ambient
        auto ambient = m_SpotLight->GetAmbientColor();
        float ambientColor[4] = { ambient.x, ambient.y, ambient.z, ambient.w };
        if (ImGui::ColorEdit4("Ambient", ambientColor))
        {
            m_SpotLight->SetAmbient(ambientColor[0], ambientColor[1], ambientColor[2], ambientColor[3]);
        }

        // Diffuse
        auto diffuse = m_SpotLight->GetDiffuseColor();
        float diffuseColor[4] = { diffuse.x, diffuse.y, diffuse.z, diffuse.w };
        if (ImGui::ColorEdit4("Diffuse", diffuseColor))
        {
            m_SpotLight->SetDiffuseColor(diffuseColor[0], diffuseColor[1], diffuseColor[2], diffuseColor[3]);
        }

        // Specular
        auto specular = m_SpotLight->GetSpecularColor();
        float specularColor[4] = { specular.x, specular.y, specular.z, specular.w };
        if (ImGui::ColorEdit4("Specular", specularColor))
        {
            m_SpotLight->SetSpecularColor(specularColor[0], specularColor[1], specularColor[2], specularColor[3]);
        }

        // Specular Power
        float power = m_SpotLight->GetSpecularPower();
        if (ImGui::SliderFloat("Specular Power", &power, 1.0f, 128.0f))
        {
            m_SpotLight->SetSpecularPower(power);
        }

        // Position
        auto pos = m_SpotLight->GetPosition();
        float position[3] = { pos.x, pos.y, pos.z };
        if (ImGui::DragFloat3("Position", position, 0.1f))
        {
            m_SpotLight->SetPosition(position[0], position[1], position[2]);
        }

        // Direction
        auto dir = m_SpotLight->GetDirection();
        float direction[3] = { dir.x, dir.y, dir.z };
        if (ImGui::DragFloat3("Direction", direction, 0.05f, -1.0f, 1.0f))
        {
            m_SpotLight->SetDirection(direction[0], direction[1], direction[2]);
        }

        // Range
        float range = m_SpotLight->GetRange();
        if (ImGui::SliderFloat("Range", &range, 0.1f, 100.0f))
        {
            m_SpotLight->SetRange(range);
        }

        // Spot Angle (in degrees)
        m_SpotLightUpdate = m_SpotLight->GetSpotAngleDegree();
        if (ImGui::SliderFloat("Spot Angle", &m_SpotLightUpdate, 1.0f, 90.0f))
        {
            m_SpotLight->SetSpotAngleDegrees(m_SpotLightUpdate);
        }
    }

    ImGui::End();
}

void TestApplication::DirectionalLightControl()
{
    ImGui::Begin("Directional Light Editor");

    if (m_DirectionalLight)
    {
        // Ambient
        auto ambient = m_DirectionalLight->GetAmbientColor();
        float ambientColor[4] = { ambient.x, ambient.y, ambient.z, ambient.w };
        if (ImGui::ColorEdit4("Ambient", ambientColor))
        {
            m_DirectionalLight->SetAmbient(ambientColor[0], ambientColor[1], ambientColor[2], ambientColor[3]);
        }

        // Diffuse
        auto diffuse = m_DirectionalLight->GetDiffuseColor();
        float diffuseColor[4] = { diffuse.x, diffuse.y, diffuse.z, diffuse.w };
        if (ImGui::ColorEdit4("Diffuse", diffuseColor))
        {
            m_DirectionalLight->SetDiffuseColor(diffuseColor[0], diffuseColor[1], diffuseColor[2], diffuseColor[3]);
        }

        // Specular
        auto specular = m_DirectionalLight->GetSpecularColor();
        float specularColor[4] = { specular.x, specular.y, specular.z, specular.w };
        if (ImGui::ColorEdit4("Specular", specularColor))
        {
            m_DirectionalLight->SetSpecularColor(specularColor[0], specularColor[1], specularColor[2], specularColor[3]);
        }

        // Specular Power
        float power = m_DirectionalLight->GetSpecularPower();
        if (ImGui::SliderFloat("Specular Power", &power, 1.0f, 128.0f))
        {
            m_DirectionalLight->SetSpecularPower(power);
        }

        // Direction
        auto dir = m_DirectionalLight->GetDirection();
        float direction[3] = { dir.x, dir.y, dir.z };
        if (ImGui::DragFloat3("Direction", direction, 0.01f, -1.0f, 1.0f))
        {
            m_DirectionalLight->SetDirection(direction[0], direction[1], direction[2]);
        }

        // Normalize direction to avoid light bugs
        if (ImGui::Button("Normalize Direction"))
        {
            DirectX::XMVECTOR v = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(direction)));
            DirectX::XMFLOAT3 normalized{};
            DirectX::XMStoreFloat3(&normalized, v);
            m_DirectionalLight->SetDirection(normalized.x, normalized.y, normalized.z);
        }
    }

    ImGui::End();
}

void TestApplication::PointLightControl()
{
    if (!m_PointLight) return;

    if (ImGui::CollapsingHeader("Point Light Settings"))
    {
        ImGui::Text("Edit Point Light Properties:");

        // Position
        DirectX::XMFLOAT3 pos = m_PointLight->GetPosition();
        if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f))
        {
            m_PointLight->SetPosition(pos.x, pos.y, pos.z);
        }

        // Range
        float range = m_PointLight->GetRange();
        if (ImGui::DragFloat("Range", &range, 0.1f, 0.1f, 100.0f))
        {
            m_PointLight->SetRange(range);
        }

        // Ambient Color
        DirectX::XMFLOAT4 ambient = m_PointLight->GetAmbientColor();
        if (ImGui::ColorEdit4("Ambient", reinterpret_cast<float*>(&ambient)))
        {
            m_PointLight->SetAmbient(ambient.x, ambient.y, ambient.z, ambient.w);
        }

        // Diffuse Color
        DirectX::XMFLOAT4 diffuse = m_PointLight->GetDiffuseColor();
        if (ImGui::ColorEdit4("Diffuse", reinterpret_cast<float*>(&diffuse)))
        {
            m_PointLight->SetDiffuseColor(diffuse.x, diffuse.y, diffuse.z, diffuse.w);
        }

        // Specular Color
        DirectX::XMFLOAT4 specular = m_PointLight->GetSpecularColor();
        if (ImGui::ColorEdit4("Specular", reinterpret_cast<float*>(&specular)))
        {
            m_PointLight->SetSpecularColor(specular.x, specular.y, specular.z, specular.w);
        }

        // Specular Power
        float specPower = m_PointLight->GetSpecularPower();
        if (ImGui::DragFloat("Specular Power", &specPower, 1.0f, 1.0f, 128.0f))
        {
            m_PointLight->SetSpecularPower(specPower);
        }
    }
}

void TestApplication::BackgroundControl()
{
    if (!m_Background) return;

    ImGui::Begin("Background Control");

    // --- Position ---
    ImGui::Text("Position");
    static float pos[3];
    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(pos), m_Background->GetRigidBody()->GetPosition());

    if (ImGui::DragFloat3("Translate", pos, 0.1f))
    {
        m_Background->GetRigidBody()->SetTranslation(pos[0], pos[1], pos[2]);
    }

    // --- Scale ---
    ImGui::Text("Scale");
    DirectX::XMFLOAT3 scale = m_Background->GetScale();
    float scaleArray[3] = { scale.x, scale.y, scale.z };

    if (ImGui::DragFloat3("Scale", scaleArray, 0.05f, 0.0f, 100.0f))
    {
        m_Background->SetScale(scaleArray[0], scaleArray[1], scaleArray[2]);
    }

    // --- Edge Percents ---
    ImGui::Text("Edge Percents");
    float left = m_Background->GetLeftPercent();
    float right = m_Background->GetRightPercent();
    float top = m_Background->GetTopPercent();
    float bottom = m_Background->GetDownPercent();

    bool changed = false;
    changed |= ImGui::SliderFloat("Left %", &left, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Right %", &right, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Top %", &top, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Down %", &bottom, 0.0f, 1.0f);

    if (changed)
    {
        m_Background->SetEdgePercents(left, right, top, bottom);
    }

    ImGui::End();
}

void TestApplication::GhostControl()
{
    if (!m_GhostSprite) return;

    ImGui::Begin("Ghost Control");

    // --- Position ---
    ImGui::Text("Position");
    static float pos[3];
    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(pos), m_GhostSprite->GetRigidBody()->GetPosition());

    if (ImGui::DragFloat3("Translate", pos, 0.1f))
    {
        m_GhostSprite->GetRigidBody()->SetTranslation(pos[0], pos[1], pos[2]);
    }

    // --- Scale ---
    ImGui::Text("Scale");
    DirectX::XMFLOAT3 scale = m_GhostSprite->GetScale();
    float scaleArray[3] = { scale.x, scale.y, scale.z };

    if (ImGui::DragFloat3("Scale", scaleArray, 0.05f, 0.0f, 100.0f))
    {
        m_GhostSprite->SetScale(scaleArray[0], scaleArray[1], scaleArray[2]);
    }

    // --- Edge Percents ---
    ImGui::Text("Edge Percents");
    float left = m_GhostSprite->GetLeftPercent();
    float right = m_GhostSprite->GetRightPercent();
    float top = m_GhostSprite->GetTopPercent();
    float bottom = m_GhostSprite->GetDownPercent();

    bool changed = false;
    changed |= ImGui::SliderFloat("Left %", &left, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Right %", &right, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Top %", &top, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Down %", &bottom, 0.0f, 1.0f);

    if (changed)
    {
        m_GhostSprite->SetEdgePercents(left, right, top, bottom);
    }

    ImGui::End();
}

void TestApplication::LeftCubeControl()
{
    if (!m_Left) return;

    ImGui::Begin("Left Cube Control");

    // --- Position ---
    ImGui::Text("Position");
    static float pos[3];
    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(pos), m_Left->GetRigidBody()->GetPosition());

    if (ImGui::DragFloat3("Translate", pos, 0.1f))
    {
        m_Left->GetRigidBody()->SetTranslation(pos[0], pos[1], pos[2]);
    }

    // --- Scale ---
    ImGui::Text("Scale");
    DirectX::XMFLOAT3 scale = m_Left->GetScale();
    float scaleArray[3] = { scale.x, scale.y, scale.z };

    if (ImGui::DragFloat3("Scale", scaleArray, 0.05f, 0.0f, 100.0f))
    {
        m_Left->SetScale(scaleArray[0], scaleArray[1], scaleArray[2]);
    }

    ImGui::End();
}
