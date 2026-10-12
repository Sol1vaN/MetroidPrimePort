#pragma once
#include <algorithm>
#include <cmath>
namespace aurora::input {
inline bool KeyboardOwnsMovement(int keyX,int keyY,int mergedX,int mergedY) {
  return (keyX!=0 || keyY!=0) && keyX==mergedX && keyY==mergedY;
}
struct KeyboardWalk {
  double value=1, velocity=0;
  bool initialized=false;
  double Update(bool active, bool alwaysRun, bool held, double walk, double run, double dt) {
    walk=std::isfinite(walk)?std::clamp(walk,.25,.50):.50;
    run=std::isfinite(run)?std::clamp(run,.70,1.0):1.0;
    if(!active || !std::isfinite(dt) || dt<=0 || dt>.25) {
      value=1;velocity=0;initialized=false;return 1;
    }
    const bool running=alwaysRun != held;
    const double target=running?run:walk;
    if(!initialized) { value=alwaysRun?run:walk;velocity=0;initialized=true; }
    // Analytic critically damped spring: smooth press/release, no overshoot.
    const double omega=6, y=value-target, c=velocity+omega*y, e=std::exp(-omega*dt);
    value=target+(y+c*dt)*e;velocity=(velocity-omega*c*dt)*e;
    return std::clamp(value,0.0,1.0);
  }
};
// Encode a fraction of the full keyboard stick AFTER the GC deadzone and
// octagonal clamp. Scaling raw 127 directly would lose low walking speeds.
inline int KeyboardWalkAxis(int axis, bool diagonal, double scale) {
  // SDL -> PAD conversion can leave a neutral Y axis at -1. Never turn
  // a value inside the native deadzone into a walking direction.
  if(std::abs(axis)<=15) return 0;
  scale=std::isfinite(scale)?std::clamp(scale,0.0,1.0):1.0;
  if(scale>=1) return axis;
  if(scale<=0) return 0;
  const double extent=diagonal?40.0/72.0:1.0;
  const int raw=15+std::max(2,int(std::ceil(112*extent*scale)));
  return axis<0?-raw:raw;
}
}
