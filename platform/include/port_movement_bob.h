#pragma once
#include <algorithm>
#include <cmath>

namespace PortMovementBob {
// Ground speed, independent of key pressure, direction, and the vertical
// velocity from jumps. A tiny noise floor still admits the 1% walk setting.
inline float SpeedFraction(float right, float forward, float maximum) {
  if (!std::isfinite(right) || !std::isfinite(forward) ||
      !std::isfinite(maximum) || maximum <= 0.f) return 0.f;
  const float fraction = std::clamp(std::hypot(right, forward) / maximum, 0.f, 1.f);
  return fraction < .001f ? 0.f : fraction;
}
inline float Cadence(float fraction) {
  return std::sqrt(std::clamp(fraction, 0.f, 1.f));
}
// Preserve the authored 60 Hz amplitude response, but make both amplitude
// and cadence continuous and tick-rate independent. Integrate the filtered
// cadence analytically so a speed change never resets the stride phase.
inline void Advance(float& amplitude, float& cadence, float& phase,
                    float targetAmplitude, float targetCadence,
                    float trackingPerTick, float dt) {
  if (!std::isfinite(dt) || dt <= 0.f) return;
  const float tracking = std::clamp(trackingPerTick, .0001f, .9999f);
  const float rate = -60.f * std::log1p(-tracking);
  const float blend = -std::expm1(-rate * dt);
  phase += targetCadence * dt + (cadence - targetCadence) * blend / rate;
  amplitude += (targetAmplitude - amplitude) * blend;
  cadence += (targetCadence - cadence) * blend;
}
} // namespace PortMovementBob
