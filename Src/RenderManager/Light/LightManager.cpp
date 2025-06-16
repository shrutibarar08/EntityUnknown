#include "LightManager.h"

#include "Utils/Logger/Logger.h"

void LightManager::SetDirectionalLightBufferSize(int bufferSize, int slot)
{
	m_DirectionalBufferSize = bufferSize;
	m_DirectionalBufferSlot = slot;
}

void LightManager::SetSpotLightBufferSize(int bufferSize, int slot)
{
	m_SpotLightBufferSize = bufferSize;
	m_SpotLightBufferSlot = slot;
}

void LightManager::AddLight(ILightSource* light) const
{
	if (!m_Initialized) return;
	if (DirectionalLight* d_light = dynamic_cast<DirectionalLight*>(light))
	{
		m_DirectionalLightManager->AddLight(d_light);
	}
	if (SpotLight* s_light = dynamic_cast<SpotLight*>(light))
	{
		m_SpotLightManager->AddLight(s_light);
	}
}

void LightManager::RemoveLight(ILightSource* light) const
{
	if (!m_Initialized) return;

	if (DirectionalLight* d_light = dynamic_cast<DirectionalLight*>(light))
	{
		m_DirectionalLightManager->RemoveLight(d_light->GetAssignedID());
	}
	if (SpotLight* s_light = dynamic_cast<SpotLight*>(light))
	{
		m_SpotLightManager->RemoveLight(s_light->GetAssignedID());
	}
}

void LightManager::Clear() const
{
	if (!m_Initialized) return;

	m_DirectionalLightManager->Clear();
	m_SpotLightManager->Clear();
}

void LightManager::Build(ID3D11Device* device)
{
	if (m_Initialized) return;
	m_DirectionalLightManager = std::make_unique<DirectionalLightManager>(m_DirectionalBufferSlot, m_DirectionalBufferSize);
	m_DirectionalLightManager->Build(device);

	m_SpotLightManager = std::make_unique<SpotLightManager>(m_SpotLightBufferSize, m_SpotLightBufferSlot);
	m_SpotLightManager->Build(device);

	m_Initialized = true;
}

void LightManager::Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition) const
{
	if (!m_Initialized) return;

	m_DirectionalLightManager->Update(context);
	m_SpotLightManager->Update(context, ownerPosition);
}

void LightManager::Bind(ID3D11DeviceContext* context) const
{
	if (!m_Initialized) return;

	m_DirectionalLightManager->Bind(context);
	m_SpotLightManager->Bind(context);
}

LIGHT_META_DATA LightManager::GetLightMetaDataInfo() const
{
	if (!m_Initialized) return {};
	return {
		m_DirectionalLightManager->GetLightCount(),
		m_SpotLightManager->GetLightCount(),
	};
}
