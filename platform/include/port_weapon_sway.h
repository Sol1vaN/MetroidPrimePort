#pragma once
#include <algorithm>
#include <array>
#include <cmath>

// Camera-local presentation only: +right/+up follow the look gesture.
// AVP-inspired soft targets and analytic springs; tuned for Samus's arm cannon.
namespace PortWeaponSway {
// Retain the saved IDs of the existing styles.
enum Mode { None = -1, Stick = 0, Inertia = 1 };
constexpr double kPi = 3.14159265358979323846;
struct Spring {
  double x = 0, v = 0;
  void Step(double target, double omega, double damping, double dt) {
    const double y = x - target;
    if (damping >= 1.0) {
      const double c = v + omega * y, e = std::exp(-omega * dt);
      x = target + (y + c * dt) * e;
      v = (v - omega * c * dt) * e;
    } else {
      const double a = omega * damping, w = omega * std::sqrt(1.0 - damping * damping);
      const double c = std::cos(w * dt), s = std::sin(w * dt), e = std::exp(-a * dt);
      x = target + e * (y * c + (v + a * y) * s / w);
      v = e * (v * c - (a * v + omega * omega * y) * s / w);
    }
  }
};
struct Pose {
  double right = 0, up = 0;
  double yaw = 0, pitch = 0, roll = 0;
  double forward = 0, elbowWeight = 0;
};
// Camera-local rigid overlay shared by rendering and geometry tests.
// Rotate around the elbow, then apply placement: custom positioning cannot
// change the lever arm, and the rear anchor moves only by the small translation.
inline std::array<double, 12> VisualTransform(const Pose& pose,
    const std::array<double, 3>& pivot, const std::array<double, 3>& placement) {
  const double cy = std::cos(pose.yaw), sy = std::sin(pose.yaw);
  const double cp = std::cos(pose.pitch), sp = std::sin(pose.pitch);
  const double cr = std::cos(pose.roll), sr = std::sin(pose.roll);
  // Rz(yaw) * Rx(pitch) * Ry(roll), with +Y along the cannon.
  std::array<double, 12> m = {
    cy*cr-sy*sp*sr, -sy*cp, cy*sr+sy*sp*cr, 0,
    sy*cr+cy*sp*sr,  cy*cp, sy*sr-cy*sp*cr, 0,
    -cp*sr, sp, cp*cr, 0};
  const std::array<double, 3> shift = {pose.right, pose.forward, pose.up};
  for (int i=0; i<3; ++i)
    m[4*i+3] = pivot[i] + placement[i] + shift[i] -
        (m[4*i]*pivot[0] + m[4*i+1]*pivot[1] + m[4*i+2]*pivot[2]);
  return m;
}
struct State {
  Spring horizontal, vertical, lookPointing, stickBlend;
  double previousYaw = 0, previousPitch = 0;
  bool initialized = false;
  void Reset() { *this = {}; }
  Pose GetPose(double strength = 1.0) const {
    // Metroid: X right, Y forward, Z up. Positive Z rotation looks left.
    const double gesture = stickBlend.x;
    Pose pose;
    pose.right = horizontal.x * (.10 - .05 * gesture);
    pose.up = vertical.x * (.08 - .04 * gesture) + lookPointing.x * .015;
    pose.yaw = -horizontal.x * (.85 + 1.55 * gesture);
    pose.pitch = vertical.x * (.75 + 1.45 * gesture) + lookPointing.x;
    pose.roll = -horizontal.x * (.20 - .08 * gesture);
    // Stick points around the authored elbow; only a small amount of arm travel.
    pose.forward = -std::hypot(horizontal.x, vertical.x) * .015 * gesture;
    pose.elbowWeight = initialized ? 1.0 : 0.0;
    // Strength changes the gesture, never the anchor or spring timing.
    strength = std::isfinite(strength) ? std::clamp(strength, 0.0, 2.0) : 1.0;
    strength *= 1.3 - .5 * gesture; // Inertia 100%=old 130%; Stick 100%=old 80%.
    pose.right *= strength; pose.up *= strength; pose.forward *= strength;
    pose.yaw *= strength; pose.pitch *= strength; pose.roll *= strength;
    return pose;
  }
  Pose Update(bool active, int mode, double dt, double yaw, double pitch) {
    if (!active || (mode != Stick && mode != Inertia) || !std::isfinite(dt) || dt < 0 || dt > .15 ||
        !std::isfinite(yaw) || !std::isfinite(pitch)) { Reset(); return {}; }
    if (!initialized) {
      previousYaw = yaw; previousPitch = pitch; initialized = true;
      return GetPose();
    }
    if (dt == 0) return GetPose();
    const double rightRate = -std::remainder(yaw - previousYaw, 2 * kPi) / dt;
    const double upRate = (pitch - previousPitch) / dt;
    previousYaw = yaw; previousPitch = pitch;
    const bool inertia = mode == Inertia;
    const double limit = inertia ? .095 : .18;
    // Inertia uses the user-selected, formerly inverted X/Y directions permanently.
    const double gain = inertia ? .045 : .065;
    double targetRight = gain * rightRate, targetUp = gain * upRate;
    // Absolute pitch pointing is distinct from turn-rate sway: it remains
    // subtly posed when looking up/down after the mouse has stopped.
    const double pointing = inertia ? std::sin(std::clamp(pitch, -1.52, 1.52)) * .055 : 0.0;
    lookPointing.Step(pointing, 6.0, 1.0, dt);
    stickBlend.Step(inertia ? 0.0 : 1.0, 20.0, 1.0, dt);
    // A radial soft limit keeps diagonal flicks as restrained as axial flicks.
    const double radius = std::hypot(targetRight, targetUp);
    if (radius > 0) {
      const double scale = limit * std::tanh(radius / limit) / radius;
      targetRight *= scale; targetUp *= scale;
    }
    // A slower critically damped Stick spring retains momentum through
    // direction changes and eases into rest without bounce or oscillation.
    const double omega = inertia ? 11.0 : 14.0;
    const double damping = inertia ? .82 : 1.0;
    horizontal.Step(targetRight, omega, damping, dt);
    vertical.Step(targetUp, omega, damping, dt);
    return GetPose();
  }
};
} // namespace PortWeaponSway
