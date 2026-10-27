#pragma once

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>

namespace rengine
{
	/// @brief Sandbox 전용 자유 이동 카메라. 입력 수집과 GPU 자원은 소유하지 않는다.
	class SandboxCamera
	{
	public:
		/// @brief 마우스 이동량(픽셀)을 yaw·pitch에 반영한다.
		/// @note 호출자는 유한한 이동량을 전달한다. yaw는 순환하고 pitch는 극점 직전으로 제한한다.
		void Rotate(float deltaX, float deltaY)
		{
			constexpr float sensitivity = 0.003f;
			constexpr float pitchLimit = DirectX::XM_PIDIV2 - 0.01f;
			mYaw = std::remainder(mYaw + deltaX * sensitivity, DirectX::XM_2PI);
			mPitch = std::clamp(mPitch - deltaY * sensitivity, -pitchLimit, pitchLimit);
		}

		/// @brief 전후는 시선 방향, 좌우는 수평, 상하는 월드 Y축. 대각선 이동도 같은 속도로 맞춘다.
		/// @param seconds 유한한 0 이상의 이동 시간(초). Sandbox 프레임 루프는 0~0.1초를 전달한다.
		/// @note right·up·forward는 유한한 -1~1 입력이다. 입력 검증·시간 제한은 호출자가 담당한다.
		void Move(float right, float up, float forward, float seconds)
		{
			const DirectX::XMVECTOR direction = DirectX::XMVectorAdd(
				DirectX::XMVectorScale(GetForward(), forward),
				DirectX::XMVectorSet(std::cos(mYaw) * right, up, -std::sin(mYaw) * right, 0.0f));
			if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(direction)) > 0.0f)
			{
				constexpr float moveSpeed = 3.0f;
				const DirectX::XMVECTOR displacement = DirectX::XMVectorScale(
					DirectX::XMVector3Normalize(direction),
					moveSpeed * seconds);
				DirectX::XMStoreFloat3(
					&mPosition,
					DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&mPosition), displacement));
			}
		}

		DirectX::XMFLOAT3 GetPosition() const
		{
			return mPosition;
		}

		/// @brief 유한한 월드 위치를 설정한다. 잘못된 값은 기존 위치를 유지하고 false를 반환한다.
		[[nodiscard]] bool SetPosition(const DirectX::XMFLOAT3& position)
		{
			if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
			{
				return false;
			}

			mPosition = position;
			return true;
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
}
