#include "TestApplication.h"

#include <random>

#include "External/Imgui/imgui.h"

bool TestApplication::InitializeApplication(const SweetLoader& sweetLoader)
{
	m_Ground = std::make_unique<ModelCube>();
	m_Ground->GetRigidBody()->SetTranslation(0, -2, 1);
	DirectX::XMVECTOR scale_ground{ 25, 1, 25 };
	m_Ground->GetCubeCollider()->SetScale(scale_ground);
	m_Ground->SetScale(scale_ground);
	m_Ground->GetShaderResource()->SetTexture("Texture/sample.tga");
    m_Ground->GetShaderResource()->SetSecondaryTexture("Texture/test.tga");
	m_Ground->GetCubeCollider()->SetColliderState(ColliderState::Static);
	Render3DQueue::AddModel(m_Ground.get());

	DirectX::XMVECTOR scale_right{ 2, 1, 1 };

    m_GhostSprite = std::make_unique<WorldSpaceSprite>();
    m_GhostSprite->GetShaderResource()->SetTexture("Texture/ghost-2.tga");
    m_GhostSprite->SetScale(2.0f, 2.0f, 1.f);
    Render2DQueue::AddSpaceSprite(m_GhostSprite.get());

    m_GrassSprite = std::make_unique<WorldSpaceSprite>();
    m_GrassSprite->GetShaderResource()->SetTexture("Texture/grass-front.tga");
    m_GrassSprite->SetScale(2.0f, 2.0f, 1.f);
    m_GrassSprite->GetCubeCollider()->SetColliderState(ColliderState::Static);
    Render2DQueue::AddSpaceSprite(m_GrassSprite.get());

	m_DirectionalLight = std::make_unique<DirectionalLight>();
	m_DirectionalLight->SetDiffuseColor(1.f, 1.f, 1.f, 1.0f);
	m_DirectionalLight->SetDirection(-0.545f, -0.209f, 0.812f);
	m_DirectionalLight->SetAmbient(0.f, 0.f, 0.f, 1.0f);
	m_DirectionalLight->SetSpecularColor(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirectionalLight->SetSpecularPower(128.0f);
	Render3DQueue::AddLight(m_DirectionalLight.get());

	m_SpotLight = std::make_unique<SpotLight>();
	m_SpotLight->SetAmbient(0.f, 0.f, 0.f, 1.0f);
	m_SpotLight->SetDiffuseColor(38.0f / 255.f, 88.f / 255.f, 14.f / 255.f, 1.0f);
	m_SpotLight->SetSpecularColor(1.0f, 1.f, 1.f, 1.0f);
	m_SpotLight->SetSpecularPower(128.0f);
	m_SpotLight->SetPosition(-4.0f, 8.8f, -6.8f);
	m_SpotLight->SetDirection(0.400f, -1.0f, 1.f);
	m_SpotLight->SetRange(21.0f);
	m_SpotLight->SetSpotAngleDegrees(24.344f);
	Render3DQueue::AddLight(m_SpotLight.get());

    m_PointLight = std::make_unique<PointLight>();
    m_PointLight->SetAmbient(124.f / 255.f, 73.f / 255.f, 73.f / 255.f, 1.0f);
    m_PointLight->SetDiffuseColor(1.0f, 0.8f, 0.6f, 1.0f);
    m_PointLight->SetSpecularColor(1.0f, 0.95f, 0.9f, 1.0f);
    m_PointLight->SetSpecularPower(128.f);
    m_PointLight->SetPosition(0.0f, 3.0f, 3.0f);
    m_PointLight->SetRange(8.0f);
    Render3DQueue::AddLight(m_PointLight.get());

    Render2DQueue::AddLight(m_PointLight.get());
    Render2DQueue::AddLight(m_SpotLight.get());
    Render2DQueue::AddLight(m_DirectionalLight.get());

    m_Background = std::make_unique<BackgroundSprite>();
    m_Background->GetRigidBody()->SetTranslation(0, 269, 0);
    m_Background->SetEdgePercents(1.f, 1.f, 1.f, 1.f);
    m_Background->GetShaderResource()->SetTexture("Texture/test.tga");
    m_Background->GetRigidBody()->SetYaw(1.57f);
    m_Background->SetTopPercent(0.631f);
    Render2DQueue::AddBackgroundSprite(m_Background.get());

    m_IdleMan = std::make_unique<WorldSpaceSprite>();
    m_IdleMan->GetCubeCollider()->SetColliderState(ColliderState::Dynamic);
    DirectX::XMVECTOR idleManScale{ 1.0f, 0.75f, 1.0f };
    m_IdleMan->GetCubeCollider()->SetScale(idleManScale);
    m_IdleMan->SetScale(2.5f, 2.5f, 2.0f);
    m_IdleMan->GetRigidBody()->AddTranslation(0.145f, -0.26, 2.0f);
    Render2DQueue::AddSpaceSprite(m_IdleMan.get());

    m_IdleManAnim = std::make_unique<SpriteAnim>(m_IdleMan.get());

    std::string basePath = "Texture/idle/0_Reaper_Man_Idle_0";
    for (int i = 1; i <= 17; i++)
    {
        std::string path;
        if (i < 10)
        {
            path = basePath + "0" + std::to_string(i) + ".tga";
        }else
        {
            path = basePath + std::to_string(i) + ".tga";
        }
        LOG_INFO("Frame Path: " + path);
        m_IdleManAnim->AddFrame(path);
    }
    m_IdleManAnim->Build(m_RenderSystem->GetDevice(), m_RenderSystem->GetDeviceContext());
	m_IdleManAnim->Play();

	m_Timer.Reset();
	return true;
}

void TestApplication::Update()
{
    float deltaTime = m_Timer.Tick();
    m_IdleManAnim->Update(deltaTime);
}

void TestApplication::RenderBegin()
{
    if (ImGui::CollapsingHeader("Lights & Object Controls", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::CollapsingHeader("Directional Light")) DirectionalLightControl();
        if (ImGui::CollapsingHeader("Spot Light")) SpotLightControl();
        if (ImGui::CollapsingHeader("Point Light")) PointLightControl();
        if (ImGui::CollapsingHeader("Background")) BackgroundControl();
        if (ImGui::CollapsingHeader("Ghost")) GhostControl();
        if (ImGui::CollapsingHeader("Idle Man")) IdleManControl();
        if (ImGui::CollapsingHeader("Grass Sprite")) GrassSpriteControl();
    }
}

void TestApplication::RenderExecute()
{
}

void TestApplication::RenderEnd()
{
}

void TestApplication::SpotLightControl()
{
    if (!m_SpotLight) return;

    if (ImGui::CollapsingHeader("Spot Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
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
}

void TestApplication::DirectionalLightControl()
{
    if (!m_DirectionalLight) return;

    if (ImGui::CollapsingHeader("Directional Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
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
}

void TestApplication::PointLightControl()
{
    if (!m_PointLight) return;

    if (ImGui::CollapsingHeader("Point Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Edit Point Light Properties:");

        // === Position ===
        DirectX::XMFLOAT3 pos = m_PointLight->GetPosition();
        if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f))
        {
            m_PointLight->SetPosition(pos.x, pos.y, pos.z);
        }

        // === Range ===
        float range = m_PointLight->GetRange();
        if (ImGui::DragFloat("Range", &range, 0.1f, 0.1f, 100.0f))
        {
            m_PointLight->SetRange(range);
        }

        // === Ambient Color ===
        DirectX::XMFLOAT4 ambient = m_PointLight->GetAmbientColor();
        if (ImGui::ColorEdit4("Ambient", reinterpret_cast<float*>(&ambient)))
        {
            m_PointLight->SetAmbient(ambient.x, ambient.y, ambient.z, ambient.w);
        }

        // === Diffuse Color ===
        DirectX::XMFLOAT4 diffuse = m_PointLight->GetDiffuseColor();
        if (ImGui::ColorEdit4("Diffuse", reinterpret_cast<float*>(&diffuse)))
        {
            m_PointLight->SetDiffuseColor(diffuse.x, diffuse.y, diffuse.z, diffuse.w);
        }

        // === Specular Color ===
        DirectX::XMFLOAT4 specular = m_PointLight->GetSpecularColor();
        if (ImGui::ColorEdit4("Specular", reinterpret_cast<float*>(&specular)))
        {
            m_PointLight->SetSpecularColor(specular.x, specular.y, specular.z, specular.w);
        }

        // === Specular Power ===
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

    if (ImGui::CollapsingHeader("Background Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Edit Background Properties:");

        // === Position ===
        float pos[3];
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(pos), m_Background->GetRigidBody()->GetPosition());

        if (ImGui::DragFloat3("Position", pos, 0.1f))
        {
            m_Background->GetRigidBody()->SetTranslation(pos[0], pos[1], pos[2]);
        }

        // === Scale ===
        DirectX::XMFLOAT3 scale = m_Background->GetScale();
        float scaleArray[3] = { scale.x, scale.y, scale.z };

        if (ImGui::DragFloat3("Scale", scaleArray, 0.05f, 0.0f, 100.0f))
        {
            m_Background->SetScale(scaleArray[0], scaleArray[1], scaleArray[2]);
        }

        // === Edge Percents ===
        float left = m_Background->GetLeftPercent();
        float right = m_Background->GetRightPercent();
        float top = m_Background->GetTopPercent();
        float bottom = m_Background->GetDownPercent();

        bool changed = false;
        changed |= ImGui::SliderFloat("Left %", &left, 0.0f, 1.0f);
        changed |= ImGui::SliderFloat("Right %", &right, 0.0f, 1.0f);
        changed |= ImGui::SliderFloat("Top %", &top, 0.0f, 1.0f);
        changed |= ImGui::SliderFloat("Bottom %", &bottom, 0.0f, 1.0f);

        if (changed)
        {
            m_Background->SetEdgePercents(left, right, top, bottom);
        }
    }
}

void TestApplication::GhostControl()
{
    if (!m_GhostSprite) return;

    if (ImGui::CollapsingHeader("Ghost Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Edit Ghost Sprite Properties:");

        // === Position ===
        float pos[3];
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(pos), m_GhostSprite->GetRigidBody()->GetPosition());

        if (ImGui::DragFloat3("Position", pos, 0.1f))
        {
            m_GhostSprite->GetRigidBody()->SetTranslation(pos[0], pos[1], pos[2]);
        }

        // === Scale ===
        DirectX::XMFLOAT3 scale = m_GhostSprite->GetScale();
        float scaleArray[3] = { scale.x, scale.y, scale.z };

        if (ImGui::DragFloat3("Scale", scaleArray, 0.05f, 0.0f, 100.0f))
        {
            m_GhostSprite->SetScale(scaleArray[0], scaleArray[1], scaleArray[2]);
        }
    }
}

void TestApplication::IdleManControl()
{
    if (!m_IdleMan) return;

    if (ImGui::CollapsingHeader("IdleMan Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("IdleMan Transform Controls:");

        // === Position ===
        float pos[3];
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(pos),
            m_IdleMan->GetRigidBody()->GetPosition());

        if (ImGui::DragFloat3("Position", pos, 0.1f))
        {
            m_IdleMan->GetRigidBody()->SetTranslation(pos[0], pos[1], pos[2]);
        }

        // === Model Scale ===
        ImGui::Separator();
        ImGui::Text("Model Scale");
        DirectX::XMFLOAT3 modelScale = m_IdleMan->GetScale();
        float modelScaleArray[3] = { modelScale.x, modelScale.y, modelScale.z };

        if (ImGui::DragFloat3("Model Scale", modelScaleArray, 0.05f, 0.0f, 100.0f))
        {
            m_IdleMan->SetScale(modelScaleArray[0], modelScaleArray[1], modelScaleArray[2]);
        }

        // === Collider Scale ===
        ImGui::Separator();
        ImGui::Text("Collider Scale");
        DirectX::XMVECTOR colliderScale = m_IdleMan->GetCubeCollider()->GetScale();
        DirectX::XMFLOAT3 colliderScaleFloat3;
        DirectX::XMStoreFloat3(&colliderScaleFloat3, colliderScale);
        float colliderScaleArray[3] = {
            colliderScaleFloat3.x,
            colliderScaleFloat3.y,
            colliderScaleFloat3.z
        };

        if (ImGui::DragFloat3("Collider Scale", colliderScaleArray, 0.05f, 0.0f, 100.0f))
        {
            DirectX::XMVECTOR vec = DirectX::XMVectorSet(
                colliderScaleArray[0],
                colliderScaleArray[1],
                colliderScaleArray[2],
                0.0f
            );
            m_IdleMan->GetCubeCollider()->SetScale(vec);
        }
    }
}

void TestApplication::GrassSpriteControl()
{
    if (!m_GrassSprite) return;

    if (ImGui::CollapsingHeader("Grass Sprite Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // === Position ===
        float pos[3];
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(pos),
            m_GrassSprite->GetRigidBody()->GetPosition());

        if (ImGui::DragFloat3("Position", pos, 0.1f))
        {
            m_GrassSprite->GetRigidBody()->SetTranslation(pos[0], pos[1], pos[2]);
        }

        // === Model Scale ===
        ImGui::Separator();
        ImGui::Text("Model Scale");
        DirectX::XMFLOAT3 modelScale = m_GrassSprite->GetScale();
        float modelScaleArray[3] = { modelScale.x, modelScale.y, modelScale.z };

        if (ImGui::DragFloat3("Model Scale", modelScaleArray, 0.05f, 0.0f, 100.0f))
        {
            m_GrassSprite->SetScale(modelScaleArray[0], modelScaleArray[1], modelScaleArray[2]);
        }

        // === Collider Scale ===
        ImGui::Separator();
        ImGui::Text("Collider Scale");
        DirectX::XMVECTOR colliderScale = m_GrassSprite->GetCubeCollider()->GetScale();
        DirectX::XMFLOAT3 colliderScaleFloat3;
        DirectX::XMStoreFloat3(&colliderScaleFloat3, colliderScale);
        float colliderScaleArray[3] = {
            colliderScaleFloat3.x,
            colliderScaleFloat3.y,
            colliderScaleFloat3.z
        };

        if (ImGui::DragFloat3("Collider Scale", colliderScaleArray, 0.05f, 0.0f, 100.0f))
        {
            DirectX::XMVECTOR vec = DirectX::XMVectorSet(
                colliderScaleArray[0],
                colliderScaleArray[1],
                colliderScaleArray[2],
                0.0f
            );
            m_GrassSprite->GetCubeCollider()->SetScale(vec);
        }
    }
}
