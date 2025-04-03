#include "Camera.h"

#define TO_RADIANS(angle) angle * 0.0174532925f

Camera::Camera()
{
	m_PositionX = 0.0f;
	m_PositionY = 0.0f;
	m_PositionZ = 0.0f;

	m_RotationX = 0.0f;
	m_RotationY = 0.0f;
	m_RotationZ = 0.0f;

	m_LookDir = { 0.f, 0.f, 1.f };
}

Camera::Camera(const Camera&)
{
}

Camera::~Camera()
{
}

void Camera::SetPosition(float x, float y, float z)
{
	m_PositionX = x;
	m_PositionY = y;
	m_PositionZ = z;
}

void Camera::SetRotation(float Pitch, float Yaw)
{
	m_RotationX = Pitch;
	m_RotationY = Yaw;
}

void Camera::SetLookDir(float x, float y, float z)
{
	m_LookDir = { x, y, z };
	DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&m_LookDir);
	v = DirectX::XMVector3Normalize(v);
	DirectX::XMStoreFloat3(&m_LookDir, v);
}

void Camera::Render()
{
	DirectX::XMFLOAT3 Up, Position;
	DirectX::XMVECTOR UpVector, PositionVector, LookAtVector, ReversePositionVector;
	DirectX::XMMATRIX RotationMatrix;
	float Yaw, Pitch, Roll;

	Up.x = 0.f;
	Up.y = 1.f;
	Up.z = 0.f;

	UpVector = DirectX::XMLoadFloat3(&Up);

	Position.x = m_PositionX;
	Position.y = m_PositionY;
	Position.z = m_PositionZ;

	PositionVector = DirectX::XMLoadFloat3(&Position);

	Position.x = -Position.x;
	Position.y = -Position.y;
	Position.z = -Position.z;

	ReversePositionVector = DirectX::XMLoadFloat3(&Position);

	LookAtVector = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_LookDir));

	Pitch = TO_RADIANS(m_RotationX);
	Yaw = TO_RADIANS(m_RotationY);
	Roll = TO_RADIANS(0.f);

	RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(Pitch, Yaw, Roll);

	LookAtVector = DirectX::XMVector3TransformCoord(LookAtVector, RotationMatrix);
	UpVector = DirectX::XMVector3TransformCoord(UpVector, RotationMatrix);

	LookAtVector = DirectX::XMVectorAdd(PositionVector, LookAtVector);

	//m_ViewMatrix = DirectX::XMMatrixLookAtLH(PositionVector, LookAtVector, UpVector);

	//m_ViewMatrix = DirectX::XMMatrixLookAtLH(PositionVector, ReversePositionVector, UpVector);
	m_ViewMatrix = DirectX::XMMatrixLookAtLH(PositionVector, LookAtVector, UpVector);
}

DirectX::XMFLOAT3 Camera::GetRotatedLookDir() const
{
	DirectX::XMVECTOR LookVector = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_LookDir));
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(TO_RADIANS(m_RotationX), TO_RADIANS(m_RotationY), 0.f);
	
	LookVector = DirectX::XMVector3TransformCoord(LookVector, RotationMatrix);
	LookVector = DirectX::XMVector3Normalize(LookVector);

	DirectX::XMFLOAT3 RotatedDir;
	DirectX::XMStoreFloat3(&RotatedDir, LookVector);
	return RotatedDir;
}

DirectX::XMFLOAT3 Camera::GetRotatedLookRight() const
{
	DirectX::XMVECTOR LookVector = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_LookDir));
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(TO_RADIANS(m_RotationX), TO_RADIANS(m_RotationY), 0.f);

	LookVector = DirectX::XMVector3TransformCoord(LookVector, RotationMatrix);
	LookVector = DirectX::XMVector3Normalize(LookVector);

	DirectX::XMFLOAT3 Up = { 0.f, 1.f, 0.f };
	DirectX::XMVECTOR UpVector = DirectX::XMLoadFloat3(&Up);
	DirectX::XMVECTOR RightVector = DirectX::XMVector3Cross(LookVector, UpVector);

	DirectX::XMFLOAT3 RotatedLookRight;
	DirectX::XMStoreFloat3(&RotatedLookRight, RightVector);
	return RotatedLookRight;
}
