#pragma once
#include <algorithm>
#include <cmath>
#include <musyx/pc_map_step_timing.h>
namespace PortMapMath {
// Smooth an accumulated displacement, not a velocity: no mouse events or wheel
// notches are lost when render and simulation rates differ.
inline double Drain(double& pending, double dt, double rate = 18.0) {
  if (!std::isfinite(pending) || !std::isfinite(dt) || dt > .25) { pending = 0; return 0; }
  if (dt <= 0) return 0;
  double result = pending * -std::expm1(-rate * dt);
  pending -= result;
  if (std::abs(pending) < 1e-9) { result += pending; pending = 0; }
  return result;
}
// Audio only: hysteresis rejects tiny movement; input expiry silences the
// smoothing tail immediately. The visual displacement is never modified.
struct SoundGate {
  bool playing=false;
  bool Update(float displacement,float dt,bool recent,float startSpeed,float stopSpeed) {
    if(!recent || !std::isfinite(displacement) || !(dt>0) || dt>.25f) return playing=false;
    const float speed=std::abs(displacement)/dt;
    playing=speed>(playing?stopSpeed:startSpeed);
    return playing;
  }
};
// Low-speed cadence follows filtered mouse travel continuously. Normal movement
// reaches the measured native rhythm; faster motion approaches, but never exceeds,
// 130%. Keep the established very-slow cadence, then extend the slow range
// with a smooth logarithmic response before the native rhythm is reached.
inline float RotationTeeth(float yaw,float pitch,float dt,bool recent) {
  if(!recent || !std::isfinite(yaw) || !std::isfinite(pitch) || !(dt>0) || dt>.1f)return 0;
  const double speed=std::hypot(yaw,pitch)/dt;
  // Below .02 rad/s the tooth spacing is exactly the same as before.
  const double slowSpeed=speed<=.02?speed:.02+.015*std::log1p((speed-.02)/.015);
  const double nativeSpeed=.02+.015*std::expm1(2.0);
  const double headroom=MUSYX_MAP_STEP_MAX_RATIO-1.0;
  // Match derivatives at both joins: no cadence jump when leaving slow motion.
  const double fastRiseTime=.05*headroom*std::exp(2.0);
  const double ratio=slowSpeed<=.05?slowSpeed/.05:
      1.0+headroom*-std::expm1(-(speed-nativeSpeed)/fastRiseTime);
  return float(MUSYX_MAP_STEP_NATIVE_HZ*std::min(ratio,MUSYX_MAP_STEP_MAX_RATIO)*dt);
}
inline bool Exit(bool mouse, bool z, bool b) { return z || (!mouse && b); }
// Camera focus must move against the requested movement of the map itself.
struct Pan { float x, z; };
inline Pan Move(bool mouse, float lx, float ly, float rx, float ry, bool invert = false) {
  const float direction = invert ? 1.f : -1.f;
  return mouse ? Pan{direction * lx, direction * ly} : Pan{direction * rx, direction * ry};
}
}
