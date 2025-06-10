#include "CameraController.h"
#include "Utils/Logger/Logger.h"

#include <cmath>
#include <format>
#include <algorithm>

using namespace DirectX;

CameraController::CameraController(int id, const std::string& name)
    : m_id(id), m_name(name)
{
    mCameraEyePosition = XMVectorSet(0.0f, 1.0f, -10.0f, 0.0f);
    mCameraLookingAt = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    mCameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    mCameraRotationQuaternion = XMQuaternionIdentity();
}

int CameraController::GetID() const
{
    return m_id;
}

std::string CameraController::GetName() const
{
    return m_name;
}

void CameraController::SetTranslationX(float x)
{
    mCameraEyePosition = XMVectorSetX(mCameraEyePosition, x);
}

void CameraController::AddTranslationX(float x)
{
    mCameraEyePosition = DirectX::XMVectorSetX(mCameraEyePosition, x + GetTranslationX());
}

float CameraController::GetTranslationX() const
{
    return DirectX::XMVectorGetX(mCameraEyePosition);
}

void CameraController::SetTranslationY(float y)
{
    mCameraEyePosition = DirectX::XMVectorSetY(mCameraEyePosition, y);
}

void CameraController::AddTranslationY(float y)
{
    mCameraEyePosition = DirectX::XMVectorSetY(mCameraEyePosition, y + GetTranslationY());
}

float CameraController::GetTranslationY() const
{
    return DirectX::XMVectorGetY(mCameraEyePosition);
}

void CameraController::SetTranslationZ(float z)
{
    mCameraEyePosition = DirectX::XMVectorSetZ(mCameraEyePosition, z);
}

void CameraController::AddTranslationZ(float z)
{
    mCameraEyePosition = DirectX::XMVectorSetZ(mCameraEyePosition, z + GetTranslationZ());
}

float CameraController::GetTranslationZ() const
{
    return DirectX::XMVectorGetZ(mCameraEyePosition);
}

void CameraController::AddTranslation(int axis, float value)
{
    if (axis == 0) AddTranslationX(value);
    else if (axis == 1) AddTranslationY(value);
    else if (axis == 2) AddTranslationZ(value);
}

void CameraController::Rotate(int axis, float value)
{
    if (axis == 0) RotatePitch(value);
    else if (axis == 1) RotateYaw(value);
    else if (axis == 2) RotateRoll(value);
}

DirectX::XMFLOAT3 CameraController::GetRotationAngles() const
{
    XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(mCameraLookingAt, mCameraEyePosition));

    // Extract Yaw (rotation around Y-axis)
    float yaw = atan2(XMVectorGetX(forward), XMVectorGetZ(forward));

    // Extract Pitch (rotation around X-axis)
    float pitch = asin(-XMVectorGetY(forward)); // Invert Y to align with pitch movement

    XMVECTOR right = XMVector3Normalize(XMVector3Cross(mCameraUp, forward));
    float roll = atan2(
        XMVectorGetY(right),
        XMVectorGetX(right)
    );

    return XMFLOAT3(pitch, yaw, roll);
}

DirectX::XMMATRIX CameraController::GetProjectionMatrix() const
{
    if (mAspectRatio <= 0.0f) return XMMatrixIdentity();

    float nearZ = 0.1f;
    float farZ = (mFarZ > nearZ) ? mFarZ : nearZ + 10.0f;

    return XMMatrixPerspectiveFovLH(mFOV, mAspectRatio, nearZ, farZ);
}

DirectX::XMMATRIX CameraController::GetOrthogonalMatrix() const
{
    if (mAspectRatio <= 0.0f) return XMMatrixIdentity();

    float nearZ = 0.1f;
    float farZ = (mFarZ > nearZ) ? mFarZ : nearZ + 10.0f;
    float orthoHeight = 10.0f;

    return XMMatrixOrthographicLH(mAspectRatio * orthoHeight, orthoHeight, nearZ, farZ);
}

void CameraController::SetMaxVisibleDistance(float farZ)
{
    mFarZ = min(1.f, farZ);
}

float CameraController::GetMaxVisibleDistance() const
{
    return mFarZ;
}

void CameraController::SetAspectRatio(float ratio)
{
    mAspectRatio = ratio;
}

float CameraController::GetAspectRatio() const
{
    return mAspectRatio;
}

void CameraController::MoveForward(float delta)
{
    XMVECTOR forward = GetForwardVector();
    mCameraEyePosition = XMVectorAdd(mCameraEyePosition, XMVectorScale(forward, delta * mSpeed));
}

void CameraController::MoveRight(float delta)
{
    XMVECTOR right = GetRightVector();
    mCameraEyePosition = XMVectorAdd(mCameraEyePosition, XMVectorScale(right, delta * mSpeed));
}

void CameraController::MoveUp(float delta)
{
    XMVECTOR up = GetUpVector();
    mCameraEyePosition = XMVectorAdd(mCameraEyePosition, XMVectorScale(up, delta * mSpeed));
}

void CameraController::RotateYaw(float angle)
{
    XMVECTOR rotation = XMQuaternionRotationAxis(mCameraUp, angle);
    mCameraRotationQuaternion = XMQuaternionMultiply(mCameraRotationQuaternion, rotation);
}

void CameraController::RotatePitch(float angle)
{
    XMVECTOR rotation = XMQuaternionRotationAxis(GetRightVector(), angle);
    mCameraRotationQuaternion = XMQuaternionMultiply(mCameraRotationQuaternion, rotation);
}

void CameraController::RotateRoll(float angle)
{
    XMVECTOR forward = GetForwardVector();
    XMVECTOR rotation = XMQuaternionRotationAxis(forward, angle);

    mCameraRotationQuaternion = XMQuaternionMultiply(rotation, mCameraRotationQuaternion);
}

DirectX::XMMATRIX CameraController::GetViewMatrix() const
{
    DirectX::XMVECTOR forward = GetForwardVector(); // rotated
    DirectX::XMVECTOR up = GetUpVector();           // rotated
    DirectX::XMVECTOR lookAtPosition = DirectX::XMVectorAdd(mCameraEyePosition, forward);

    return DirectX::XMMatrixLookAtLH(mCameraEyePosition, lookAtPosition, up);
}

void CameraController::SetFieldOfView(float fov)
{
    mFOV = fov;
}

float CameraController::GetFieldOfView() const
{
    return mFOV;
}

void CameraController::SetMovementSpeed(float speed)
{
    mSpeed = speed;
}

float CameraController::GetMovementSpeed() const
{
    return mSpeed;
}

DirectX::XMVECTOR CameraController::GetForwardVector() const
{
    return XMVector3Rotate(XMVectorSet(0, 0, 1, 0), mCameraRotationQuaternion);
}

DirectX::XMVECTOR CameraController::GetRightVector() const
{
    return XMVector3Rotate(XMVectorSet(1, 0, 0, 0), mCameraRotationQuaternion);
}

DirectX::XMVECTOR CameraController::GetUpVector() const
{
    return XMVector3Rotate(XMVectorSet(0, 1, 0, 0), mCameraRotationQuaternion);
}

CameraManager::CameraManager()
    : m_activeCamera(nullptr), m_nextID(1)
{
}

int CameraManager::AddCamera(const std::string& name)
{
    // Generate unique ID with mutex protection
    int id = m_nextID++;

    // Create and add the new CameraController
    std::unique_ptr<CameraController> cam = std::make_unique<CameraController>(id, name);

    m_cameras.push_back(std::move(cam));
    // If no active camera, set this as active
    if (!m_activeCamera)
    {
        m_activeCamera = m_cameras.back().get();
    }

    LOG_INFO("Added Camera Component: " + name);

    return id;
}

bool CameraManager::RemoveCamera(int id)
{
    for (auto it = m_cameras.begin(); it != m_cameras.end(); ++it)
    {
        if ((*it)->GetID() == id)
        {
            if (m_activeCamera == it->get()) m_activeCamera = nullptr;
            m_cameras.erase(it);
            return true;
        }
    }
    return false; // Not found
}

CameraController* CameraManager::GetCamera(int id) const
{
    for (const auto& camPtr : m_cameras)
    {
        if (camPtr->GetID() == id)
        {
            CameraController* cam = camPtr.get();
            return cam;
        }
    }
    return nullptr;
}

CameraController* CameraManager::GetCameraByName(const std::string& name) const
{
    for (const auto& camPtr : m_cameras)
    {
        if (camPtr->GetName() == name)
        {
            CameraController* cam = camPtr.get();
            return cam;
        }
    }

    return nullptr;
}

void CameraManager::SetActiveCamera(int id)
{
    for (const auto& camPtr : m_cameras)
    {
        if (camPtr->GetID() == id)
        {
            m_activeCamera = camPtr.get();
            break;
        }
    }
}

CameraController* CameraManager::GetActiveCamera() const
{
    return m_activeCamera;
}