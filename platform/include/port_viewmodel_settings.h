#pragma once
#include <algorithm>
#include <cmath>
namespace PortViewmodelSettings {
struct Position { float x, y, z; };
struct Settings {
  bool enabled = false;
  bool automatic = true;
  float fov = 55.f;
  float right = 25.f, forward = 30.f, up = -35.f;
  float Fov(float cameraFov) const {
    return !enabled ? 55.f : automatic ? cameraFov : fov;
  }
  Position GetPosition(float cameraFov) const {
    if (!enabled) return {.25f, .30f, -.35f};
    if (!automatic) return {right * .01f, forward * .01f, up * .01f};
    // PrimeHack ViewModifier::adjust_viewmodel: automatic arm placement.
    const float t = std::clamp((cameraFov - 75.f) / 50.f, 0.f, 1.f);
    return {.25f - .03f * t, .30f - .32f * t, -.35f};
  }
};
// Visor particles are authored against the retail 4:3, 55-degree camera.
inline Position VisorScale(float aspect, float fov) {
  if (!std::isfinite(aspect) || aspect <= 0.f || !std::isfinite(fov) || fov <= 0.f || fov >= 179.f)
    return {1.f, 1.f, 1.f};
  const float vertical = std::tan(fov * .00872664626f) / std::tan(55.f * .00872664626f);
  return {vertical * aspect / (4.f / 3.f), 1.f, vertical};
}
} // namespace PortViewmodelSettings
