//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Camera.h"

namespace DXMapEditer
{
	Camera::Camera()
		: m_Position(0.0f, 0.0f, 0.0f),
		m_Right(1.0f, 0.0f, 0.0f),
		m_Up(0.0f, 1.0f, 0.0f),
		m_Look(0.0f, 0.0f, 1.0f)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPosition(float x, float y, float z)
	{
		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
	}

	void Camera::SetPosition(const XMFLOAT3& v)
	{
		m_Position = v;
	}

	float Camera::GetFovX() const
	{
		float halfWidth = 0.5f*GetNearWindowWidth();
		return (float)2.0f * atanf(halfWidth / m_NearZ);
	}

	void Camera::SetLens(float fovY, float aspect, float zn, float zf)
	{
		// cache properties
		m_FovY = fovY;
		m_Aspect = aspect;
		m_NearZ = zn;
		m_FarZ = zf;

		m_NearWindowHeight = 2.0f * m_NearZ * tanf(0.5f * m_FovY);
		m_FarWindowHeight = 2.0f * m_FarZ * tanf(0.5f * m_FovY);

		XMMATRIX P = XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
		XMStoreFloat4x4(&m_Proj, P);
	}

	void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
	{
		XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
		XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
		XMVECTOR U = XMVector3Cross(L, R);

		XMStoreFloat3(&m_Position, pos);
		XMStoreFloat3(&m_Look, L);
		XMStoreFloat3(&m_Right, R);
		XMStoreFloat3(&m_Up, U);
	}

	void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
	{
		XMVECTOR P = XMLoadFloat3(&pos);
		XMVECTOR T = XMLoadFloat3(&target);
		XMVECTOR U = XMLoadFloat3(&up);

		LookAt(P, T, U);
	}

	void Camera::Strafe(float d)
	{
		// mPosition += d*mRight
		XMVECTOR s = XMVectorReplicate(d);
		XMVECTOR r = XMLoadFloat3(&m_Right);
		XMVECTOR p = XMLoadFloat3(&m_Position);
		XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, r, p));
	}

	void Camera::Walk(float d)
	{
		// mPosition += d*mLook
		XMVECTOR s = XMVectorReplicate(d);
		XMVECTOR l = XMLoadFloat3(&m_Look);
		XMVECTOR p = XMLoadFloat3(&m_Position);
		XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, l, p));
	}

	void Camera::Pitch(float angle)
	{
		// Rotate up and look vector about the right vector.

		XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), angle);

		XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
		XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));
	}

	void Camera::RotateY(float angle)
	{
		// Rotate the basis vectors about the world y-axis.

		XMMATRIX R = XMMatrixRotationY(angle);

		XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), R));
		XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
		XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));
	}

	void Camera::UpdateViewMatrix()
	{
		XMVECTOR R = XMLoadFloat3(&m_Right);
		XMVECTOR U = XMLoadFloat3(&m_Up);
		XMVECTOR L = XMLoadFloat3(&m_Look);
		XMVECTOR P = XMLoadFloat3(&m_Position);

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&m_Right, R);
		XMStoreFloat3(&m_Up, U);
		XMStoreFloat3(&m_Look, L);

		m_View(0, 0) = m_Right.x;
		m_View(1, 0) = m_Right.y;
		m_View(2, 0) = m_Right.z;
		m_View(3, 0) = x;

		m_View(0, 1) = m_Up.x;
		m_View(1, 1) = m_Up.y;
		m_View(2, 1) = m_Up.z;
		m_View(3, 1) = y;

		m_View(0, 2) = m_Look.x;
		m_View(1, 2) = m_Look.y;
		m_View(2, 2) = m_Look.z;
		m_View(3, 2) = z;

		m_View(0, 3) = 0.0f;
		m_View(1, 3) = 0.0f;
		m_View(2, 3) = 0.0f;
		m_View(3, 3) = 1.0f;
	}

	void Camera::SetMoveSpeed(const int moveSpeed)
	{
		m_MoveSpeed = static_cast<float>(moveSpeed) * 0.001f;
	}

	void Camera::GoOrigin(int clientWidth, int clientHeight)
	{
		SetPosition(0.0f, 50.0f, -8.0f);
		SetLens(XM_PIDIV2, (clientWidth / (float)clientHeight), 0.3f, 1000.0f);
		UpdateViewMatrix();
	}

}