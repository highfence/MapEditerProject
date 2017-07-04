//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   -It keeps track of the camera coordinate system relative to the world space
//    so that the view matrix can be constructed.  
//   -It keeps track of the viewing frustum of the camera so that the projection
//    matrix can be obtained.
//***************************************************************************************

#pragma once

namespace DXMapEditer 
{
#include <DirectXMath.h>
	using namespace DirectX;

	class Camera
	{
	public:
		Camera();
		~Camera();

		// Get/Set world camera position.
		XMVECTOR GetPositionXM() const { return XMLoadFloat3(&m_Position); };
		XMFLOAT3 GetPosition() const { return m_Position; };
		void SetPosition(float x, float y, float z);
		void SetPosition(const XMFLOAT3& v);

		// Get camera basis vectors.
		XMVECTOR GetRightXM() const { return XMLoadFloat3(&m_Right); };
		XMFLOAT3 GetRight() const { return m_Right; };
		XMVECTOR GetUpXM() const { return XMLoadFloat3(&m_Up); };
		XMFLOAT3 GetUp() const { return m_Up; };
		XMVECTOR GetLookXM() const { return XMLoadFloat3(&m_Look); };
		XMFLOAT3 GetLook() const { return m_Look; };

		// Get frustum properties.
		float GetNearZ() const { return m_NearZ; };
		float GetFarZ() const { return m_FarZ; };
		float GetAspect() const { return m_Aspect; };
		float GetFovY() const { return m_FovY; };
		float GetFovX() const;

		// Get near and far plane dimensions in view space coordinates.
		float GetNearWindowWidth() const { return m_Aspect * m_NearWindowHeight; };
		float GetNearWindowHeight() const { return m_NearWindowHeight; };
		float GetFarWindowWidth() const { return m_Aspect * m_FarWindowHeight; };
		float GetFarWindowHeight() const { return m_FarWindowHeight; };

		// Set frustum.
		void SetLens(float fovY, float aspect, float zn, float zf);

		// Define camera space via LookAt parameters.
		void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
		void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

		// Get View/Proj matrices.
		XMMATRIX GetView() const { return XMLoadFloat4x4(&m_View); };
		XMFLOAT4X4 GetView4x4f() const { return m_View; };
		XMMATRIX GetProj() const { return XMLoadFloat4x4(&m_Proj); };
		XMFLOAT4X4 GetProj4x4f() const { return m_Proj; };
		XMMATRIX GetViewProj() const { return XMMatrixMultiply(GetView(), GetProj()); };
		

		// Strafe/Walk the camera a distance d.
		void Strafe(float d);
		void Walk(float d);

		// Rotate the camera.
		void Pitch(float angle);
		void RotateY(float angle);

		// After modifying camera position/orientation, call to rebuild the view matrix.
		void UpdateViewMatrix();

		float GetMoveSpeed() const { return m_MoveSpeed; };

	private:

		// Camera coordinate system with coordinates relative to world space.
		XMFLOAT3 m_Position;
		XMFLOAT3 m_Right;
		XMFLOAT3 m_Up;
		XMFLOAT3 m_Look;

		// Cache frustum properties.
		float m_NearZ;
		float m_FarZ;
		float m_Aspect;
		float m_FovY;
		float m_NearWindowHeight;
		float m_FarWindowHeight;

		// Cache View/Proj matrices.
		XMFLOAT4X4 m_View;
		XMFLOAT4X4 m_Proj;

		float m_MoveSpeed = 0.1f;
	};

}