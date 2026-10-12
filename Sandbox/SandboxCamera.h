#pragma once

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>

// Sandbox 전용 자유 이동 카메라. 입력 수집과 GPU 자원은 소유하지 않는다.
class SandboxCamera
{
public:
	/// @brief 마우스 이동량(픽셀)을 yaw·pitch에 반영한다.
	void Rotate(float deltaX, float deltaY)
	{
		constexpr float sensitivity = 0.003f;
		constexpr float pitchLimit = DirectX::XM_PIDIV2 - 0.01f;
		mYaw = std::remainder(mYaw + deltaX * sensitivity, DirectX::XM_2PI);
		mPitch = std::clamp(mPitch - deltaY * sensitivity, -pitchLimit, pitchLimit);
	}

	/// @brief 전후는 시선 방향, 좌우는 수평, 상하는 월드 Y축. 대각선 이동도 같은 속도로 맞춘다.
	/// @param seconds 이동에 반영할 시간(초).
	void Move(float right, float up, float forward, float seconds)
	{
		const auto direction = DirectX::XMVectorAdd(
			DirectX::XMVectorScale(GetForward(), forward),
			DirectX::XMVectorSet(std::cos(mYaw) * right, up, -std::sin(mYaw) * right, 0.0f));
		if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(direction)) > 0.0f)
		{
			constexpr float moveSpeed = 3.0f;
			const auto displacement = DirectX::XMVectorScale(
				DirectX::XMVector3Normalize(direction),
				moveSpeed * seconds);
			DirectX::XMStoreFloat3(&mPosition, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&mPosition), displacement));
		}
	}

	DirectX::XMFLOAT3 GetPosition() const
	{
		return mPosition;
	}

	DirectX::XMMATRIX GetView() const
	{
		return DirectX::XMMatrixLookToLH(
			DirectX::XMLoadFloat3(&mPosition),
			GetForward(),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

private:
	DirectX::XMVECTOR GetForward() const
	{
		return DirectX::XMVectorSet(
			std::sin(mYaw) * std::cos(mPitch), std::sin(mPitch), std::cos(mYaw) * std::cos(mPitch), 0.0f);
	}

	DirectX::XMFLOAT3 mPosition{ 0.0f, 2.0f, -5.0f };
	float mYaw = 0.0f;
	float mPitch = -std::atan2(2.0f, 5.0f);
};
