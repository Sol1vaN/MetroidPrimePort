#pragma once
#include <algorithm>
#include <cmath>
namespace PortFeedback {
struct Quat { double w=1, x=0, y=0, z=0; };
inline Quat Normalize(Quat q) {
  const double n=std::sqrt(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
  return n>1e-12?Quat{q.w/n,q.x/n,q.y/n,q.z/n}:Quat{};
}
inline Quat Blend(Quat a, Quat b, double t) {
  a=Normalize(a);b=Normalize(b);
  double dot=a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z;
  if(dot<0) { b={-b.w,-b.x,-b.y,-b.z};dot=-dot; }
  double sa=1-t,sb=t;
  if(dot<.9995) {
    const double angle=std::acos(std::clamp(dot,0.0,1.0));
    sa=std::sin((1-t)*angle)/std::sin(angle);sb=std::sin(t*angle)/std::sin(angle);
  }
  return Normalize({sa*a.w+sb*b.w,sa*a.x+sb*b.x,sa*a.y+sb*b.y,sa*a.z+sb*b.z});
}
// Previous look direction expressed in today's camera coordinates. Use cross
// and atan2, rather than acos(dot), to retain very small mouse movements.
inline Quat LagTarget(double x,double y,double z,double dt,bool invert) {
  if(!std::isfinite(dt)||dt<=0||dt>.25) return {};
  const double length=std::sqrt(x*x+y*y+z*z);
  if(length<1e-9)return {};
  x/=length;y/=length;z/=length;
  const double sine=std::hypot(x,z),angle=std::atan2(sine,y);
  if(sine<1e-12 || angle>1.0) return {}; // camera cut: no kick
  const double half=std::min(angle/(60*dt),.3)*(invert?-1:1);
  const double scale=std::sin(half)/sine;
  return {std::cos(half),z*scale,0,-x*scale};
}
inline Quat LagStep(Quat current,Quat target,double freeLookSpeed,double dt) {
  if(!std::isfinite(dt)||dt<=0||dt>.25)return current;
  // Native angular rate at 60 Hz, now per second rather than proportional to dt squared.
  current=Normalize(current);target=Normalize(target);
  const Quat delta{current.w*target.w+current.x*target.x+current.y*target.y+current.z*target.z,
    current.w*target.x-current.x*target.w-current.y*target.z+current.z*target.y,
    current.w*target.y+current.x*target.z-current.y*target.w-current.z*target.x,
    current.w*target.z-current.x*target.y+current.y*target.x-current.z*target.w};
  const double angle=2*std::atan2(std::sqrt(delta.x*delta.x+delta.y*delta.y+delta.z*delta.z),std::abs(delta.w));
  // Small input samples used to snap straight to their target, making packet
  // noise visible at fixed 60 Hz. Filter all angles, including subpixel motion.
  const double smooth = -std::expm1(-14.0 * dt);
  const double rateLimit = angle>1e-12?std::clamp(.15*freeLookSpeed*dt/angle,0.0,1.0):1;
  return Blend(current,target,std::min(smooth,rateLimit));
}
// Timer stores normalized stride phase, retaining remainder at arbitrary tick
// rates. Emit at most one sound per update and discard backlog after a stall.
inline bool Footstep(float& phase,float dt,float interval) {
  if(!std::isfinite(dt)||dt<=0||dt>.25||!std::isfinite(interval)||interval<=0) {
    if(dt>.25)phase=0;return false;
  }
  phase+=dt/interval;
  if(phase+1e-5f<1)return false;
  phase=std::fmax(0.f,phase-std::floor(phase+1e-5f));return true;
}
}
