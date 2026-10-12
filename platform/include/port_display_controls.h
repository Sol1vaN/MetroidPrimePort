#pragma once
#include <algorithm>
#include <cmath>
namespace PortDisplayControls {
constexpr float kSensitivityMin = 0.000001f;
constexpr float kSensitivityMax = 0.02f;
// Match the retail endpoints and neutral midpoint, without its nine steps.
inline float BrightnessGain(float percent) {
  if (!std::isfinite(percent)) return 1.f;
  return 0.625f + std::clamp(percent, 0.f, 100.f) * 0.0075f;
}
inline float MoveSlider(float value, int direction, float dt, float min, float max) {
  if (!std::isfinite(dt) || dt <= 0.f) return value;
  return std::clamp(value + direction * std::min(dt, 0.1f) * (max - min) * 0.4f, min, max);
}
} // namespace PortDisplayControls
