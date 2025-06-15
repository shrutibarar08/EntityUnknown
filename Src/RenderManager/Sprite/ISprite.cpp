#include "ISprite.h"


void ISprite::EnableLight(bool flag)
{
	m_LightEnabled = flag;
}

void ISprite::AddLight(ILightAnyType* lightSource)
{
	m_LightBufferManager.AddLightToAll(lightSource);
}

void ISprite::RemoveLight(ILightAnyType* lightSource)
{
	m_LightBufferManager.RemoveLightFromAll(lightSource);
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

bool ISprite::Build(ID3D11Device* device)
{
	m_LightBufferManager.RegisterBuffer<DirectionalBufferConfig>(0);
	m_LightBufferManager.BuildAll(device);

	if (!m_bInitializedStaticBuffer)
	{
		m_bInitializedStaticBuffer = true;
		m_LightMetaCB = std::make_unique<ConstantBuffer<PIXEL_LIGHT_META_GPU>>(device);
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
			PIXEL_LIGHT_META_GPU meta{};
			meta.DirectionalLightCount = 10;
			m_LightMetaCB->Update(deviceContext, &meta);

			//~ Set as a primary (slot 0) pixel constant buffer contains only light metadata
			deviceContext->PSSetConstantBuffers(0u, 1u, m_LightMetaCB->GetAddressOf());

			//~ Attach Light Sources data into the struct array (GPU Side).
			DirectX::XMFLOAT3 position = m_RigidBody.GetTranslation();
			m_LightBufferManager.RenderAll(position, deviceContext);
		}else
		{
			PIXEL_LIGHT_META_GPU meta{};
			meta.DirectionalLightCount = 0;
			m_LightMetaCB->Update(deviceContext, &meta);
			deviceContext->PSSetConstantBuffers(0u, 1u, m_LightMetaCB->GetAddressOf());
		}
	}

	return true;
}
