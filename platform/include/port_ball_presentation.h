#pragma once
#include "port_feedback.h"
#include <cstdint>

namespace PortBallPresentation {
// Positions are model centers, not the player collision origin beneath the ball.
struct Pose {
  double x=0, y=0, z=0;
  PortFeedback::Quat rotation{};
};
class History {
  Pose previous{};
  uint32_t generation=0;
  bool active=false;
public:
  void Snapshot(const Pose& pose, uint32_t tick, bool morphed) {
    previous=pose; generation=tick; active=morphed;
  }
  bool Present(const Pose& current, uint32_t tick, bool morphed, double t, Pose& out) const {
    if (!active || !morphed || generation==0 || generation!=tick ||
        !std::isfinite(t) || t<0 || t>=1) return false;
    const double dx=current.x-previous.x, dy=current.y-previous.y, dz=current.z-previous.z;
    const double travel=dx*dx+dy*dy+dz*dz;
    if (!std::isfinite(travel) || travel>16) return false; // teleport, not ordinary rolling
    // A rolling sphere can turn more than 45 degrees per tick. Do not apply
    // the ordinary actor angular-cut rule, which also discards position blending.
    out={previous.x+dx*t, previous.y+dy*t, previous.z+dz*t,
         PortFeedback::Blend(previous.rotation,current.rotation,t)};
    return true;
  }
};
}
