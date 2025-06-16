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
	m_Ground->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);
	Render3DQueue::AddModel(m_Ground.get());

	m_Right = std::make_unique<ModelCube>();
	m_Right->GetRigidBody()->SetTranslation(3, 1, 2);
	DirectX::XMVECTOR scale_right{ 2, 1, 1 };
	m_Right->GetCubeCollider()->SetScale(scale_right);
	m_Right->SetScale(scale_right);
	m_Right->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);

	Render3DQueue::AddModel(m_Right.get());

	m_Left = std::make_unique<ModelCube>();
	m_Left->GetRigidBody()->SetTranslation(-2, 1, 2);
	DirectX::XMVECTOR scale_left{ 1, 1, 1 };
	m_Left->GetCubeCollider()->SetScale(scale_left);
	m_Left->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);
	m_Left->SetTexturePath("Texture/sample.tga");

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

	m_Timer.Reset();
	return true;
}

void TestApplication::Update()
{
	float deltaTime = m_Timer.Tick();

	m_Left->GetRigidBody()->AddYaw(deltaTime);
	m_Right->GetRigidBody()->AddYaw(-deltaTime);
}

void TestApplication::RenderBegin()
{
    SpotLightControl();
    DirectionalLightControl();
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
