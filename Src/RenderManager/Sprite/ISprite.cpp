#include "ISprite.h"


void ISprite::EnableLight(bool flag)
{
	m_LightEnabled = flag;
}

void ISprite::AddLight(ILightSource* lightSource) const
{
	m_LightManager.AddLight(lightSource);
}

void ISprite::RemoveLight(ILightSource* lightSource) const
{
	m_LightManager.RemoveLight(lightSource);
}

void ISprite::UpdateTextureResource(const TEXTURE_RESOURCE& resource)
{
}

void ISprite::SetVertexShaderPath(const std::wstring& shaderPath)
{
	m_VertexShaderPath = shaderPath;
}

void ISprite::SetPixelShaderPath(const std::wstring& shaderPath)
{
	m_PixelShaderPath = shaderPath;
}

void ISprite::SetTexturePath(const std::string& texturePath)
{
	m_TexturePath = texturePath;
}

void ISprite::SetOptionalTexturePath(const std::string& texturePath)
{
	m_OptionalTexturePath = texturePath;
}

bool ISprite::Build(ID3D11Device* device)
{
	m_LightManager.Build(device);

	if (!m_bInitializedStaticBuffer)
	{
		m_bInitializedStaticBuffer = true;
		m_LightMetaCB = std::make_unique<ConstantBuffer<PIXEL_BUFFER_METADATA_GPU>>(device);
		m_WorldMatrixCB = std::make_unique<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>>(device);
	}
	return true;
}

bool ISprite::Render(ID3D11DeviceContext* deviceContext)
{
	if (m_bInitializedStaticBuffer)
	{
		//~ Set Primary Constant Buffer for every sprite
		//~ Contains world matrix and camera information.
		m_WorldMatrixCB->Update(deviceContext, &m_WorldMatrix);
		deviceContext->VSSetConstantBuffers(
			0u, 1u,
			m_WorldMatrixCB->GetAddressOf()
		);

		if (m_LightEnabled)
		{
			//~ Updates Light Meta data
			LIGHT_META_DATA data = m_LightManager.GetLightMetaDataInfo();
			PIXEL_BUFFER_METADATA_GPU gpuData{};
			gpuData.SpotLightCount = data.SpotLightCount;
			gpuData.DirectionalLightCount = data.DirectionLightCount;
			gpuData.PointLightCount = data.PointLightCount;
			gpuData.MultiTexturing = IsMultiTextureEnable();
			gpuData.DebugLine = 0;
			m_LightMetaCB->Update(deviceContext, &gpuData);

			//~ Set as a primary (slot 0) pixel constant buffer contains only light metadata
			deviceContext->PSSetConstantBuffers(0u, 1u, m_LightMetaCB->GetAddressOf());

			//~ Attach Light Sources data into the struct array (GPU Side).
			m_LightManager.Update(deviceContext, m_RigidBody.GetPosition());
			m_LightManager.Bind(deviceContext);
		}else
		{
			PIXEL_BUFFER_METADATA_GPU meta{};
			meta.DirectionalLightCount = 0;
			meta.SpotLightCount = 0;
			meta.PointLightCount = 0;
			meta.DebugLine = 0;
			m_LightMetaCB->Update(deviceContext, &meta);
			deviceContext->PSSetConstantBuffers(0u, 1u, m_LightMetaCB->GetAddressOf());
		}
	}

	return true;
}

void ISprite::SetLeftPercent(float percent)
{
	if (m_LeftPercent != percent)
	{
		m_LeftPercent = percent;
		m_bDirty = true;
	}
}

void ISprite::SetRightPercent(float percent)
{
	if (m_RightPercent != percent)
	{
		m_RightPercent = percent;
		m_bDirty = true;
	}
}

void ISprite::SetTopPercent(float percent)
{
	if (m_TopPercent != percent)
	{
		m_TopPercent = percent;
		m_bDirty = true;
	}
}

void ISprite::SetDownPercent(float percent)
{
	if (m_DownPercent != percent)
	{
		m_DownPercent = percent;
		m_bDirty = true;
	}
}

float ISprite::GetLeftPercent() const
{
	return m_LeftPercent;
}

float ISprite::GetRightPercent() const
{
	return m_RightPercent;
}

float ISprite::GetTopPercent() const
{
	return m_TopPercent;
}

float ISprite::GetDownPercent() const
{
	return m_DownPercent;
}

void ISprite::SetEdgePercents(float left, float right, float top, float down)
{
	bool changed = false;

	if (m_LeftPercent != left)
	{
		m_LeftPercent = left;
		changed = true;
	}
	if (m_RightPercent != right)
	{
		m_RightPercent = right;
		changed = true;
	}
	if (m_TopPercent != top)
	{
		m_TopPercent = top;
		changed = true;
	}
	if (m_DownPercent != down)
	{
		m_DownPercent = down;
		changed = true;
	}

	if (changed)
		m_bDirty = true;
}

void ISprite::GetEdgePercents(float& left, float& right, float& top, float& down) const
{
	left = m_LeftPercent;
	right = m_RightPercent;
	top = m_TopPercent;
	down = m_DownPercent;
}
