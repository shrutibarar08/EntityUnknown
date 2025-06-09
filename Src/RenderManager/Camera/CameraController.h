#pragma once

#include <windows.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <memory>

class CameraController
{
public:
	// Constructor initializes camera with given ID and name
	CameraController(int id, const std::string& name);

	int GetID() const;
	std::string GetName() const;

	void  SetTranslationX(float x);
	void  AddTranslationX(float x);
	float GetTranslationX() const;

	void  SetTranslationY(float y);
	void  AddTranslationY(float y);
	float GetTranslationY() const;

	void  SetTranslationZ(float z);
	void  AddTranslationZ(float z);
	float GetTranslationZ() const;

	void AddTranslation(int axis, float value);
	void Rotate(int axis, float value);
	DirectX::XMFLOAT3 GetRotationAngles() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMMATRIX GetOrthogonalMatrix() const;

	void  SetMaxVisibleDistance(float farZ);
	float GetMaxVisibleDistance() const;

	void  SetAspectRatio(float ratio);
	float GetAspectRatio() const;

	void MoveForward(float delta);
	void MoveRight(float delta);
	void MoveUp(float delta);

	void RotateYaw(float angle);
	void RotatePitch(float angle);
	void RotateRoll(float angle);

	DirectX::XMMATRIX GetViewMatrix() const;

	void SetFieldOfView(float fov);
	float GetFieldOfView() const;

	void SetMovementSpeed(float speed);
	float GetMovementSpeed() const;

	DirectX::XMVECTOR GetForwardVector() const;
	DirectX::XMVECTOR GetRightVector() const;
	DirectX::XMVECTOR GetUpVector() const;

private:
	int m_id;
	std::string m_name;

	DirectX::XMVECTOR mCameraEyePosition{};
	DirectX::XMVECTOR mCameraLookingAt{};
	DirectX::XMVECTOR mCameraUp{};
	DirectX::XMVECTOR mCameraRotationQuaternion;

	float mFarZ{ 500.f };
	float mAspectRatio{ 1270.f / 720.f };
	float mSpeed{ 5.f };
	float mFOV = DirectX::XMConvertToRadians(45.f);
};

//-----------------------------------------------------------------------------
// CameraManager Class - Manages multiple CameraController instances
//-----------------------------------------------------------------------------
class CameraManager
{
public:
	CameraManager();
	~CameraManager() = default;

	int AddCamera(const std::string& name);
	bool RemoveCamera(int id);
	void SetActiveCamera(int id);

	CameraController* GetCamera(int id) const;
	CameraController* GetCameraByName(const std::string& name) const;
	CameraController* GetActiveCamera() const;

private:
	std::vector<std::unique_ptr<CameraController>> m_cameras;
	CameraController* m_activeCamera;   // Currently active camera
	int m_nextID;             // For generating unique camera IDs
};
