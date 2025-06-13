#include "CameraController.h"
#include "Utils/Logger/Logger.h"

#include <cmath>
#include <format>
#include <algorithm>

using namespace DirectX;

CameraController::CameraController(int id, const std::string& name)
    : m_id(id), m_name(name)
{
    m_CameraEyePosition = XMVectorSet(0.0f, 1.0f, -10.0f, 0.0f);
    m_CameraLookingAt = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_CameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_CameraRotationQuaternion = XMQuaternionIdentity();
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
    m_CameraEyePosition = XMVectorSetX(m_CameraEyePosition, x);
}

void CameraController::AddTranslationX(float x)
{
    m_CameraEyePosition = DirectX::XMVectorSetX(m_CameraEyePosition, x + GetTranslationX());
}

float CameraController::GetTranslationX() const
{
    return DirectX::XMVectorGetX(m_CameraEyePosition);
}

void CameraController::SetTranslationY(float y)
{
    m_CameraEyePosition = DirectX::XMVectorSetY(m_CameraEyePosition, y);
}

void CameraController::AddTranslationY(float y)
{
    m_CameraEyePosition = DirectX::XMVectorSetY(m_CameraEyePosition, y + GetTranslationY());
}

float CameraController::GetTranslationY() const
{
    return DirectX::XMVectorGetY(m_CameraEyePosition);
}

void CameraController::SetTranslationZ(float z)
{
    m_CameraEyePosition = DirectX::XMVectorSetZ(m_CameraEyePosition, z);
}

void CameraController::AddTranslationZ(float z)
{
    m_CameraEyePosition = DirectX::XMVectorSetZ(m_CameraEyePosition, z + GetTranslationZ());
}

float CameraController::GetTranslationZ() const
{
    return DirectX::XMVectorGetZ(m_CameraEyePosition);
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
    XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(m_CameraLookingAt, m_CameraEyePosition));

    // Extract Yaw (rotation around Y-axis)
    float yaw = atan2(XMVectorGetX(forward), XMVectorGetZ(forward));

    // Extract Pitch (rotation around X-axis)
    float pitch = asin(-XMVectorGetY(forward)); // Invert Y to align with pitch movement

    XMVECTOR right = XMVector3Normalize(XMVector3Cross(m_CameraUp, forward));
    float roll = atan2(
        XMVectorGetY(right),
        XMVectorGetX(right)
    );

    return XMFLOAT3(pitch, yaw, roll);
}

DirectX::XMMATRIX CameraController::GetProjectionMatrix() const
{
    if (m_AspectRatio <= 0.0f) return XMMatrixIdentity();

    float nearZ = 0.1f;
    float farZ = (m_FarZ > nearZ) ? m_FarZ : nearZ + 10.0f;

    return XMMatrixPerspectiveFovLH(m_FOV, m_AspectRatio, nearZ, farZ);
}

DirectX::XMMATRIX CameraController::GetOrthogonalMatrix() const
{
    if (m_AspectRatio <= 0.0f) return XMMatrixIdentity();

    float nearZ = 0.1f;
    float farZ = (m_FarZ > nearZ) ? m_FarZ : nearZ + 10.0f;
    float orthoHeight = 10.0f;

    return XMMatrixOrthographicLH(m_AspectRatio * orthoHeight, orthoHeight, nearZ, farZ);
}

DirectX::XMMATRIX CameraController::GetOrthogonalWindowedMatrix() const
{
    XMMATRIX m_orthoMatrix = XMMatrixOrthographicLH(
        static_cast<float>(m_WindowsScreenWidth),
        static_cast<float>(m_WindowsScreenHeight),
        m_NearZ,
        m_FarZ
    );
    return m_orthoMatrix;
}

void CameraController::SetMaxVisibleDistance(float farZ)
{
    m_FarZ = min(1.f, farZ);
}

float CameraController::GetMaxVisibleDistance() const
{
    return m_FarZ;
}

void CameraController::SetAspectRatio(float ratio)
{
    m_AspectRatio = ratio;
}

void CameraController::SetWindowsScreenSize(int width, int height)
{
    m_WindowsScreenHeight = height;
    m_WindowsScreenWidth = width;
}

int CameraController::GetWindowsScreenHeight() const
{
    return m_WindowsScreenHeight;
}

int CameraController::GetWindowsScreenWidth() const
{
    return m_WindowsScreenWidth;
}

float CameraController::GetAspectRatio() const
{
    return m_AspectRatio;
}

void CameraController::MoveForward(float delta)
{
    XMVECTOR forward = GetForwardVector();
    m_CameraEyePosition = XMVectorAdd(m_CameraEyePosition, XMVectorScale(forward, delta * m_Speed));
}

void CameraController::MoveRight(float delta)
{
    XMVECTOR right = GetRightVector();
    m_CameraEyePosition = XMVectorAdd(m_CameraEyePosition, XMVectorScale(right, delta * m_Speed));
}

void CameraController::MoveUp(float delta)
{
    XMVECTOR up = GetUpVector();
    m_CameraEyePosition = XMVectorAdd(m_CameraEyePosition, XMVectorScale(up, delta * m_Speed));
}

void CameraController::RotateYaw(float angle)
{
    XMVECTOR rotation = XMQuaternionRotationAxis(m_CameraUp, angle);
    m_CameraRotationQuaternion = XMQuaternionMultiply(m_CameraRotationQuaternion, rotation);
}

void CameraController::RotatePitch(float angle)
{
    XMVECTOR rotation = XMQuaternionRotationAxis(GetRightVector(), angle);
    m_CameraRotationQuaternion = XMQuaternionMultiply(m_CameraRotationQuaternion, rotation);
}

void CameraController::RotateRoll(float angle)
{
    XMVECTOR forward = GetForwardVector();
    XMVECTOR rotation = XMQuaternionRotationAxis(forward, angle);

    m_CameraRotationQuaternion = XMQuaternionMultiply(rotation, m_CameraRotationQuaternion);
}

DirectX::XMMATRIX CameraController::GetViewMatrix() const
{
    DirectX::XMVECTOR forward = GetForwardVector(); // rotated
    DirectX::XMVECTOR up = GetUpVector();           // rotated
    DirectX::XMVECTOR lookAtPosition = DirectX::XMVectorAdd(m_CameraEyePosition, forward);

    return DirectX::XMMatrixLookAtLH(m_CameraEyePosition, lookAtPosition, up);
}

void CameraController::SetFieldOfView(float fov)
{
    m_FOV = fov;
}

float CameraController::GetFieldOfView() const
{
    return m_FOV;
}

void CameraController::SetMovementSpeed(float speed)
{
    m_Speed = speed;
}

float CameraController::GetMovementSpeed() const
{
    return m_Speed;
}

DirectX::XMVECTOR CameraController::GetForwardVector() const
{
    return XMVector3Rotate(XMVectorSet(0, 0, 1, 0), m_CameraRotationQuaternion);
}

DirectX::XMVECTOR CameraController::GetRightVector() const
{
    return XMVector3Rotate(XMVectorSet(1, 0, 0, 0), m_CameraRotationQuaternion);
}

DirectX::XMVECTOR CameraController::GetUpVector() const
{
    return XMVector3Rotate(XMVectorSet(0, 1, 0, 0), m_CameraRotationQuaternion);
}

DirectX::XMFLOAT3 CameraController::GetEyePosition() const
{
    XMFLOAT3 position;
    XMStoreFloat3(&position, m_CameraEyePosition);
    return position;
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