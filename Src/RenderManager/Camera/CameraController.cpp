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

bool CameraController::IsLookingAtAttached() const
{
    return m_bLookAtAttached;
}

void CameraController::LookAtAttached(bool flag)
{
    m_bLookAtAttached = flag;
}

void CameraController::AttachCameraToObject(IRender* renderObj)
{
    m_AttachedTo = renderObj;
}

void CameraController::DetachCameraFromObject()
{
    m_AttachedTo = nullptr;
}

bool CameraController::IsCameraAttachedToObject() const
{
    return m_AttachedTo != nullptr;
}

IRender* CameraController::GetAttachedObject() const
{
    return m_AttachedTo;
}

bool CameraController::IsFollowingAttached() const
{
    return m_bFollowAttached;
}

void CameraController::FollowAttached(bool flag)
{
    m_bFollowAttached = flag;
}

void CameraController::SetOffsetToAttached(const DirectX::XMFLOAT3& offset)
{
    m_AttachedOffset = offset;
}

DirectX::XMFLOAT3 CameraController::GetOffsetToAttach() const
{
    return m_AttachedOffset;
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
    m_FarZ = std::min(1.f, farZ);
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
    using namespace DirectX;

    XMVECTOR eye = m_CameraEyePosition;
    XMVECTOR lookAt = XMVectorAdd(m_CameraEyePosition, GetForwardVector());
    XMVECTOR up = GetUpVector();

    if (IsCameraAttachedToObject())
    {
        XMVECTOR objectPos = GetAttachedObject()->GetRigidBody()->GetPosition();
        XMVECTOR offset = XMLoadFloat3(&m_AttachedOffset);

        if (IsFollowingAttached())
        {
            eye = XMVectorAdd(objectPos, offset);
            m_CameraEyePosition = eye;
        }

        if (IsLookingAtAttached())
        {
            lookAt = objectPos;
        }
        else if (IsFollowingAttached())
        {
            lookAt = XMVectorAdd(eye, GetForwardVector());
        }
    }

    // === prevent eye == lookAt ===
    XMVECTOR dir = XMVectorSubtract(lookAt, eye);
    if (XMVector3Equal(dir, XMVectorZero()))
    {
        dir = GetForwardVector(); // fallback to default forward
        lookAt = XMVectorAdd(eye, dir);
    }

    return XMMatrixLookAtLH(eye, lookAt, up);
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

void CameraController::SetSweetData(const SweetLoader& sweetData)
{
    // Eye position
    if (const auto& eye = sweetData["EyePosition"]; eye.IsValid())
    {
        m_CameraEyePosition = DirectX::XMVectorSet(
            eye["x"].AsFloat(),
            eye["y"].AsFloat(),
            eye["z"].AsFloat(),
            1.0f
        );
    }

    // Looking At
    if (const auto& look = sweetData["LookAt"]; look.IsValid())
    {
        m_CameraLookingAt = DirectX::XMVectorSet(
            look["x"].AsFloat(),
            look["y"].AsFloat(),
            look["z"].AsFloat(),
            1.0f
        );
    }

    // Up Vector
    if (const auto& up = sweetData["Up"]; up.IsValid())
    {
        m_CameraUp = DirectX::XMVectorSet(
            up["x"].AsFloat(),
            up["y"].AsFloat(),
            up["z"].AsFloat(),
            0.0f
        );
    }

    // Rotation Quaternion
    if (const auto& rot = sweetData["Rotation"]; rot.IsValid())
    {
        m_CameraRotationQuaternion = DirectX::XMVectorSet(
            rot["x"].AsFloat(),
            rot["y"].AsFloat(),
            rot["z"].AsFloat(),
            rot["w"].AsFloat()
        );
    }

    m_FarZ = sweetData["FarZ"].AsFloat();
    m_NearZ = sweetData["NearZ"].AsFloat();
    m_AspectRatio = sweetData["AspectRatio"].AsFloat();
    m_Speed = sweetData["Speed"].AsFloat();
    m_FOV = sweetData["FOV"].AsFloat(); // assumed already in radians
}

SweetLoader CameraController::GetSweetData() const
{
    SweetLoader data;

    // Eye Position
    {
        DirectX::XMFLOAT3 eye;
        DirectX::XMStoreFloat3(&eye, m_CameraEyePosition);

        SweetLoader eyeNode;
        eyeNode.GetOrCreate("x") = std::to_string(eye.x);
        eyeNode.GetOrCreate("y") = std::to_string(eye.y);
        eyeNode.GetOrCreate("z") = std::to_string(eye.z);
        data.GetOrCreate("EyePosition") = eyeNode;
    }

    // Looking At
    {
        DirectX::XMFLOAT3 look;
        DirectX::XMStoreFloat3(&look, m_CameraLookingAt);

        SweetLoader lookNode;
        lookNode.GetOrCreate("x") = std::to_string(look.x);
        lookNode.GetOrCreate("y") = std::to_string(look.y);
        lookNode.GetOrCreate("z") = std::to_string(look.z);
        data.GetOrCreate("LookAt") = lookNode;
    }

    // Up Vector
    {
        DirectX::XMFLOAT3 up;
        DirectX::XMStoreFloat3(&up, m_CameraUp);

        SweetLoader upNode;
        upNode.GetOrCreate("x") = std::to_string(up.x);
        upNode.GetOrCreate("y") = std::to_string(up.y);
        upNode.GetOrCreate("z") = std::to_string(up.z);
        data.GetOrCreate("Up") = upNode;
    }

    // Rotation Quaternion
    {
        DirectX::XMFLOAT4 rot;
        DirectX::XMStoreFloat4(&rot, m_CameraRotationQuaternion);

        SweetLoader rotNode;
        rotNode.GetOrCreate("x") = std::to_string(rot.x);
        rotNode.GetOrCreate("y") = std::to_string(rot.y);
        rotNode.GetOrCreate("z") = std::to_string(rot.z);
        rotNode.GetOrCreate("w") = std::to_string(rot.w);
        data.GetOrCreate("Rotation") = rotNode;
    }

    data.GetOrCreate("FarZ") = std::to_string(m_FarZ);
    data.GetOrCreate("NearZ") = std::to_string(m_NearZ);
    data.GetOrCreate("AspectRatio") = std::to_string(m_AspectRatio);
    data.GetOrCreate("Speed") = std::to_string(m_Speed);
    data.GetOrCreate("FOV") = std::to_string(m_FOV);

    return data;
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