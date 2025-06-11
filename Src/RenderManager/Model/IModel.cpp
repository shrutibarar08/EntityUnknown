#include "IModel.h"
#include "RenderManager/Components/ShaderResource/LightBuffer/LightBuffer.h"


bool IModel::Build(ID3D11Device* device)
{
	m_LightBufferManager.RegisterBuffer<DirectionalBufferConfig>(0);
	m_LightBufferManager.BuildAll(device);

	m_LightMetaCB = std::make_unique<ConstantBuffer<LightMeta>>(device);

	m_CameraBuffer = std::make_unique<ConstantBuffer<CAMERA_BUFFER>>(device);
	return BuildChild(device);
}

bool IModel::Render(ID3D11DeviceContext* deviceContext)
{
	m_CameraBuffer->Update(deviceContext, &m_CameraBufferData);

	LightMeta meta{};
	meta.gDirectionalLightCount = 10;
	m_LightMetaCB->Update(deviceContext, &meta);

	DirectX::XMFLOAT3 position; DirectX::XMStoreFloat3(&position, GetTranslationVector());
	m_LightBufferManager.RenderAll(position, deviceContext);

	deviceContext->PSSetConstantBuffers(1u, 1u, m_LightMetaCB->GetAddressOf());
	deviceContext->VSSetConstantBuffers(1u, 1u, m_CameraBuffer->GetAddressOf());
	return RenderChild(deviceContext);
}

void IModel::UpdateTransformation(const CAMERA_MATRIX_DESC* matrix)
{
	m_WorldTransform.ProjectionMatrix = matrix->ProjectionMatrix;
	m_WorldTransform.ViewMatrix = matrix->ViewMatrix;
	m_WorldTransform.WorldMatrix = DirectX::XMMatrixTranspose(GetTransform());
	m_WorldTransform.NormalMatrix = GetNormalTransform();
}

void IModel::UpdateCameraBuffer(const CAMERA_BUFFER& cameraBuffer)
{
	m_CameraBufferData = cameraBuffer;
}

WORLD_TRANSFORM IModel::GetWorldTransform() const
{
	return m_WorldTransform;
}

DirectX::XMVECTOR IModel::GetTranslationVector() const
{
	return DirectX::XMVectorSet(m_TranslationX, m_TranslationY, m_TranslationZ, 1.0f);
}

DirectX::XMVECTOR IModel::GetRotationVector() const
{
	return DirectX::XMVectorSet(m_RotationPitch, m_RotationYaw, m_RotationRoll, 1.0f);
}

DirectX::XMVECTOR IModel::GetScaleVector() const
{
	return DirectX::XMVectorSet(m_ScaleX, m_ScaleY, m_ScaleZ, 1.0f);
}

DirectX::XMMATRIX IModel::GetTransform() const
{
	using namespace DirectX;

	XMMATRIX scaleMat = XMMatrixScaling(m_ScaleX, m_ScaleY, m_ScaleZ);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_RotationPitch, m_RotationYaw, m_RotationRoll);
	XMMATRIX transMat = XMMatrixTranslation(m_TranslationX, m_TranslationY, m_TranslationZ);

	return scaleMat * rotMat * transMat;
}

DirectX::XMMATRIX IModel::GetNormalTransform() const
{
	using namespace DirectX;

	XMMATRIX scaleMat = XMMatrixScaling(m_ScaleX, m_ScaleY, m_ScaleZ);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_RotationPitch, m_RotationYaw, m_RotationRoll);

	XMMATRIX normalMat = XMMatrixTranspose(XMMatrixInverse(nullptr, scaleMat * rotMat));
	return normalMat;
}

void IModel::AddLight(ILightDataBase* lightSource)
{
	m_LightBufferManager.AddLightToAll(lightSource);
}

void IModel::RemoveLight(ILightDataBase* lightSource)
{
	m_LightBufferManager.RemoveLightFromAll(lightSource);
}

void IModel::SetScale(float x, float y, float z) { m_ScaleX = x; m_ScaleY = y; m_ScaleZ = z; }
void IModel::SetScaleX(float x) { m_ScaleX = x; }
void IModel::SetScaleY(float y) { m_ScaleY = y; }
void IModel::SetScaleZ(float z) { m_ScaleZ = z; }

void IModel::AddScale(float x, float y, float z) { m_ScaleX += x; m_ScaleY += y; m_ScaleZ += z; }
void IModel::AddScaleX(float x) { m_ScaleX += x; }
void IModel::AddScaleY(float y) { m_ScaleY += y; }
void IModel::AddScaleZ(float z) { m_ScaleZ += z; }

// --- Rotation Set/Add ---
void IModel::SetRotation(float pitch, float yaw, float roll)
{
	m_RotationPitch = pitch;
	m_RotationYaw = yaw;
	m_RotationRoll = roll;
}

void IModel::SetPitch(float pitch) { m_RotationPitch = pitch; }
void IModel::SetYaw(float yaw) { m_RotationYaw = yaw; }
void IModel::SetRoll(float roll) { m_RotationRoll = roll; }

void IModel::AddPitch(float delta) { m_RotationPitch += delta; }
void IModel::AddYaw(float delta) { m_RotationYaw += delta; }
void IModel::AddRoll(float delta) { m_RotationRoll += delta; }

// --- Translation Set/Add ---
void IModel::SetTranslation(float x, float y, float z) { m_TranslationX = x; m_TranslationY = y; m_TranslationZ = z; }
void IModel::SetTranslationX(float x) { m_TranslationX = x; }
void IModel::SetTranslationY(float y) { m_TranslationY = y; }
void IModel::SetTranslationZ(float z) { m_TranslationZ = z; }

void IModel::AddTranslation(float x, float y, float z) { m_TranslationX += x; m_TranslationY += y; m_TranslationZ += z; }
void IModel::AddTranslationX(float x) { m_TranslationX += x; }
void IModel::AddTranslationY(float y) { m_TranslationY += y; }
void IModel::AddTranslationZ(float z) { m_TranslationZ += z; }
