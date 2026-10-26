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
		/// @brief pitch 편집의 절댓값 상한(radian). 극점 직전까지 허용한다.
		static constexpr float PITCH_LIMIT = DirectX::XM_PIDIV2 - 0.01f;
		/// @brief 직접광 강도의 편집 상한. 키 입력과 UI가 공유한다.
		static constexpr float MAX_INTENSITY = 2.0f;

		/// @brief 방향·강도 입력을 시간에 반영한다. 각각 -1~1, seconds는 앱의 0~0.1초다.
		/// @note yaw는 순환, pitch는 극점 직전, intensity는 0~2로 제한한다.
		void Adjust(float horizontal, float vertical, float intensityChange, float seconds)
		{
			ApplyParameters(
				mYaw + horizontal * seconds,
				mPitch + vertical * seconds,
				mIntensity + intensityChange * seconds);
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

		float GetYaw() const
		{
			return mYaw;
		}

		float GetPitch() const
		{
			return mPitch;
		}

		/// @brief UI 편집도 키 조작과 같은 범위를 사용한다. 비유한 값은 기존 값을 유지하고 거부한다.
		/// @param yaw 월드 Y축 회전(radian). 2π 주기로 순환한다.
		/// @param pitch 고도(radian). ±(π/2 − 0.01)로 제한한다.
		/// @param intensity 직접광 강도. 0~2로 제한한다.
		/// @return 유한한 입력이면 보정해 적용하고 true, 비유한 입력이면 변경 없이 false.
		[[nodiscard]] bool SetParameters(float yaw, float pitch, float intensity)
		{
			if (!std::isfinite(yaw) || !std::isfinite(pitch) || !std::isfinite(intensity))
			{
				return false;
			}

			ApplyParameters(yaw, pitch, intensity);
			return true;
		}

	private:
		// 호출자가 유한한 입력을 보장한다. 키 입력과 UI에 같은 보정을 적용한다.
		void ApplyParameters(float yaw, float pitch, float intensity)
		{
			mYaw = std::remainder(yaw, DirectX::XM_2PI);
			mPitch = std::clamp(pitch, -PITCH_LIMIT, PITCH_LIMIT);
			mIntensity = std::clamp(intensity, 0.0f, MAX_INTENSITY);
		}

		float mYaw = 0.6f;
		float mPitch = -0.8f;
		float mIntensity = 0.8f;
	};
}
