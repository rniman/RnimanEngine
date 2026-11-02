#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>

namespace rengine
{
	/// @brief 유한한 값만 절대/상대 오차로 비교한다. 음수·비유한 허용 오차는 false다.
	/// @note |a-b| <= max(absoluteTolerance, relativeTolerance * max(|a|, |b|)). 공통 epsilon을 강제하지 않는다.
	inline bool NearlyEqual(float a, float b, float absoluteTolerance, float relativeTolerance)
	{
		if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(absoluteTolerance) ||
			!std::isfinite(relativeTolerance) || absoluteTolerance < 0 || relativeTolerance < 0)
		{
			return false;
		}

		const double magnitude = (std::max)(std::abs(static_cast<double>(a)), std::abs(static_cast<double>(b)));
		return std::abs(static_cast<double>(a) - b) <=
			(std::max)(static_cast<double>(absoluteTolerance), relativeTolerance * magnitude);
	}

	/// @brief 유한한 각도를 [-pi, pi)로 접는다. 비유한 입력은 nullopt다.
	/// @note float의 pi 표현을 주기로 사용한다. 매우 큰 입력에서 이미 손실된 각도 정밀도는 복구하지 않는다.
	[[nodiscard]] inline std::optional<float> WrapRadians(float radians)
	{
		if (!std::isfinite(radians))
		{
			return std::nullopt;
		}

		constexpr double pi = std::numbers::pi_v<float>;
		double wrapped = std::fmod(static_cast<double>(radians), 2.0 * pi);
		if (wrapped >= pi)
		{
			wrapped -= 2.0 * pi;
		}

		if (wrapped < -pi)
		{
			wrapped += 2.0 * pi;
		}

		const float result = static_cast<float>(wrapped);
		return result >= std::numbers::pi_v<float> ? -std::numbers::pi_v<float> : result;
	}

	/// @brief from에서 to까지 최단 부호 각도. 정확히 반 바퀴이면 -pi를 선택한다.
	/// @return 입력 각도가 비유한 값이면 nullopt.
	[[nodiscard]] inline std::optional<float> DeltaAngle(float from, float to)
	{
		const std::optional<float> start = WrapRadians(from);
		const std::optional<float> end = WrapRadians(to);
		if (!start || !end)
		{
			return std::nullopt;
		}

		return WrapRadians(*end - *start);
	}

	/// @brief 유한한 양 끝점 사이를 보간한다. t는 [0,1]로 제한하며 비유한 입력은 nullopt다.
	[[nodiscard]] inline std::optional<float> LerpClamped(float a, float b, float t)
	{
		if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(t))
		{
			return std::nullopt;
		}

		return std::lerp(a, b, std::clamp(t, 0.0f, 1.0f));
	}

	/// @brief 증가/감소 구간에서 위치 비율을 구해 [0,1]로 제한한다. 같은 끝점·비유한 입력은 nullopt다.
	[[nodiscard]] inline std::optional<float> InverseLerpClamped(float a, float b, float value)
	{
		if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(value) || a == b)
		{
			return std::nullopt;
		}

		const double t = (static_cast<double>(value) - a) / (static_cast<double>(b) - a);
		return static_cast<float>(std::clamp(t, 0.0, 1.0));
	}

	/// @brief 입력 구간 밖은 출력 끝점으로 제한한다. 역방향 구간도 지원한다.
	/// @return 입력 구간의 끝점이 같거나 입력 값·구간 끝점이 비유한 값이면 nullopt.
	[[nodiscard]] inline std::optional<float> RemapClamped(
		float value,
		float fromMin,
		float fromMax,
		float toMin,
		float toMax)
	{
		const std::optional<float> t = InverseLerpClamped(fromMin, fromMax, value);
		if (!t)
		{
			return std::nullopt;
		}

		return LerpClamped(toMin, toMax, *t);
	}

	/// @brief 구간 양 끝에서 기울기가 0인 보간 비율. 잘못된 구간은 nullopt다.
	[[nodiscard]] inline std::optional<float> SmoothStep(float edge0, float edge1, float value)
	{
		const std::optional<float> t = InverseLerpClamped(edge0, edge1, value);
		if (!t)
		{
			return std::nullopt;
		}

		return *t * *t * (3.0f - 2.0f * *t);
	}

	/// @brief 목표를 지나치지 않고 최대 maxDelta만큼 이동한다. 음수 이동량·비유한 입력은 nullopt다.
	[[nodiscard]] inline std::optional<float> MoveTowards(float current, float target, float maxDelta)
	{
		if (!std::isfinite(current) || !std::isfinite(target) || !std::isfinite(maxDelta) || maxDelta < 0)
		{
			return std::nullopt;
		}

		const double delta = static_cast<double>(target) - current;
		if (std::abs(delta) <= maxDelta)
		{
			return target;
		}

		return static_cast<float>(current + std::copysign(static_cast<double>(maxDelta), delta));
	}
}
