#pragma once

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>

namespace rengine
{
	/// @brief Sandbox 방향광의 각도·강도를 소유한다. 입력 수집과 GPU 자원은 소유하지 않는다.
	class SandboxLighting
	{
	public:
		/// @brief 방향·강도 입력을 시간에 반영한다. 각각 -1~1, seconds는 앱의 0~0.1초다.
		/// @note yaw는 순환, pitch는 극점 직전, intensity는 0~2로 제한한다.
		void Adjust(float horizontal, float vertical, float intensityChange, float seconds)
		{
			mYaw = std::remainder(mYaw + horizontal * seconds, DirectX::XM_2PI);
			constexpr float pitchLimit = DirectX::XM_PIDIV2 - 0.01f;
			mPitch = std::clamp(mPitch + vertical * seconds, -pitchLimit, pitchLimit);
			mIntensity = std::clamp(mIntensity + intensityChange * seconds, 0.0f, 2.0f);
		}

		/// @brief 방향과 강도를 초기 데모 값으로 복원한다.
		void Reset()
		{
			*this = SandboxLighting{};
		}

		/// @brief 빛이 진행하는 정규화된 월드 방향을 값으로 반환한다.
		DirectX::XMFLOAT3 GetDirection() const
		{
			return {
				std::sin(mYaw) * std::cos(mPitch),
				std::sin(mPitch),
				std::cos(mYaw) * std::cos(mPitch)
			};
		}

		float GetIntensity() const
		{
			return mIntensity;
		}

	private:
		float mYaw = 0.6f;
		float mPitch = -0.8f;
		float mIntensity = 0.8f;
	};
}
