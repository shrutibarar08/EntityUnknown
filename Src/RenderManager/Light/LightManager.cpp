#include "LightManager.h"

void LightManager::SetDirectionalLightBufferSize(int bufferSize, int slot)
{
	m_DirectionalBufferSize = bufferSize;
	m_DirectionalBuffer_Slot = slot;
}

void LightManager::SetSpotLightBufferSize(int bufferSize, int slot)
{
	m_SpotLightBufferSize = bufferSize;
	m_SpotLightBuffer_Slot = slot;
}

void LightManager::SetPointLightBufferSize(int bufferSize, int slot)
{
	m_PointLightBufferSize = bufferSize;
	m_PointLightBuffer_Slot = slot;
}

void LightManager::SetActiveDirectionalLight(bool flag)
{
	m_bActiveDirectionalLight = flag;
}

void LightManager::SetActiveSpotLight(bool flag)
{
	m_bActiveSpotLight = flag;
}

void LightManager::SetActivePointLight(bool flag)
{
	m_bActivePointLight = flag;
}

void LightManager::AddLight(ILightSource* light) const
{
	if (!m_Initialized) return;

	if (DirectionalLight* d_light = dynamic_cast<DirectionalLight*>(light))
	{
		if (m_bActiveDirectionalLight) m_DirectionalLightManager->AddLight(d_light);
	}
	else if (SpotLight* s_light = dynamic_cast<SpotLight*>(light))
	{
		if (m_bActiveSpotLight) m_SpotLightManager->AddLight(s_light);
	}
	else if (PointLight* p_light = dynamic_cast<PointLight*>(light))
	{
		if (m_bActivePointLight) m_PointLightManager->AddLight(p_light);
	}
}

void LightManager::RemoveLight(ILightSource* light) const
{
	if (!m_Initialized) return;

	if (DirectionalLight* d_light = dynamic_cast<DirectionalLight*>(light))
	{
		m_DirectionalLightManager->RemoveLight(d_light->GetAssignedID());
	}
	else if (SpotLight* s_light = dynamic_cast<SpotLight*>(light))
	{
		m_SpotLightManager->RemoveLight(s_light->GetAssignedID());
	}
	else if (PointLight* p_light = dynamic_cast<PointLight*>(light))
	{
		m_PointLightManager->RemoveLight(p_light->GetAssignedID());
	}
}

void LightManager::Clear() const
{
	if (!m_Initialized) return;

	if (m_bActiveDirectionalLight) m_DirectionalLightManager->Clear();
	if (m_bActiveSpotLight) m_SpotLightManager->Clear();
	if (m_bActivePointLight) m_PointLightManager->Clear();
}

void LightManager::Build(ID3D11Device* device)
{
	if (m_Initialized) return;

	if (m_bActiveDirectionalLight)
	{
		m_DirectionalLightManager = std::make_unique<DirectionalLightManager>(m_DirectionalBufferSize, m_DirectionalBuffer_Slot);
		m_DirectionalLightManager->Build(device);
	}

	if (m_bActivePointLight)
	{
		m_PointLightManager = std::make_unique<PointLightManager>(m_PointLightBufferSize, m_PointLightBuffer_Slot);
		m_PointLightManager->Build(device);
	}

	if (m_bActiveSpotLight)
	{
		m_SpotLightManager = std::make_unique<SpotLightManager>(m_SpotLightBufferSize, m_SpotLightBuffer_Slot);
		m_SpotLightManager->Build(device);
	}

	m_Initialized = true;
}

void LightManager::Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition) const
{
	if (!m_Initialized) return;

	if (m_bActiveDirectionalLight) m_DirectionalLightManager->Update(context);
	if (m_bActiveSpotLight) m_SpotLightManager->Update(context, ownerPosition);
	if (m_bActivePointLight) m_PointLightManager->Update(context, ownerPosition);
}

void LightManager::Bind(ID3D11DeviceContext* context) const
{
	if (!m_Initialized) return;

	if (m_bActiveDirectionalLight) m_DirectionalLightManager->Bind(context);
	if (m_bActiveSpotLight) m_SpotLightManager->Bind(context);
	if (m_bActivePointLight) m_PointLightManager->Bind(context);
}

LIGHT_META_DATA LightManager::GetLightMetaDataInfo() const
{
	if (!m_Initialized) return {};

	LIGHT_META_DATA data{};
	data.SpotLightCount = m_bActiveSpotLight ? m_SpotLightManager->GetLightCount() : 0;
	data.DirectionLightCount = m_bActiveDirectionalLight ? m_DirectionalLightManager->GetLightCount() : 0;
	data.PointLightCount = m_bActivePointLight ? m_PointLightManager->GetLightCount() : 0;

	return data;
}
