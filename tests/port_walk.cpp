#include "../extern/aurora/lib/keyboard_walk.hpp"
#include <dolphin/pad.h>
#include "../platform/include/port_mouse.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
using namespace aurora::input;
void Check(bool ok,const char* why) { if(!ok) { std::fprintf(stderr,"Walk: %s\n",why);std::abort(); } }
int main() {
  for(int hz:{30,60,120,240}) {
    KeyboardWalk ramp;
    const double dt=1.0/hz;
    Check(ramp.Update(true,true,false,.35,1,dt)==1,"always run default");
    double previous=1;
    for(int i=0;i<hz*3;++i) {
      const double v=ramp.Update(true,true,true,.35,1,dt);
      Check(v<=previous+1e-9 && v>=.35 && previous-v<.15,"smooth bounded slowdown");previous=v;
    }
    Check(std::abs(previous-.35)<1e-5,"walk reaches selected speed");
    for(int i=0;i<hz*3;++i) {
      const double v=ramp.Update(true,true,false,.35,1,dt);
      Check(v>=previous-1e-9 && v<=1 && v-previous<.15,"smooth run release");previous=v;
    }
    Check(std::abs(previous-1)<1e-5,"run reaches full speed");
    ramp.Update(false,true,false,.35,1,dt);
    Check(ramp.Update(true,false,false,.35,1,dt)==.35,"unchecked Always run defaults to walk");
    for(int i=0;i<hz*3;++i)ramp.Update(true,false,true,.35,.8,dt);
    Check(std::abs(ramp.value-.8)<1e-5,"Run bind uses its own speed");
    Check(ramp.Update(false,false,true,.35,1,dt)==1,"menus/focus loss bypass modifier");
  }
  KeyboardWalk coarse,fine;
  coarse.Update(true,true,false,.35,1,.01);fine.Update(true,true,false,.35,1,.01);
  for(int i=0;i<6;++i)coarse.Update(true,true,true,.35,1,1.0/30);
  for(int i=0;i<48;++i)fine.Update(true,true,true,.35,1,1.0/240);
  Check(std::abs(coarse.value-fine.value)<1e-10,"tick-rate independent transition");
  // Exercise Aurora's actual deadzone/octagon clamp, not a duplicate formula.
  for(bool diagonal:{false,true})for(int sign:{-1,1})for(double speed:{.01,.05,.15,.35,.7,.99,1.0}) {
    PADStatus pad[4]{};
    pad[0].stickX=KeyboardWalkAxis(sign*127,diagonal,speed);
    pad[0].stickY=diagonal?pad[0].stickX:0;
    PADClamp(pad);
    const double expected=(diagonal?40:72)*speed;
    Check(std::abs(double(pad[0].stickX)*sign-expected)<=1.01,"selected speed survives actual PADClamp");
    Check(!diagonal || pad[0].stickX==pad[0].stickY,"diagonal symmetry");
  }
  Check(KeyboardOwnsMovement(20,0,20,0),"keyboard source detected");
  Check(!KeyboardOwnsMovement(20,0,50,0) && !KeyboardOwnsMovement(20,0,20,30) &&
        !KeyboardOwnsMovement(0,0,0,0),"controller takeover and idle bypass keyboard correction");
  // Integrate the same friction -> force -> velocity sequence used by the
  // player, across speeds, axes and tick rates. Verify actual speed, not just PAD values.
  for(int hz:{30,60,120,240})for(float fraction:{.01f,.05f,.15f,.35f})
    for(auto direction:{PortMouse::Planar{1,0},PortMouse::Planar{0,-1},PortMouse::Planar{1,1}}) {
      const float dt=1.f/hz,mass=90.f,maximum=12.f,acceleration=1000.f;
      PortMouse::Planar velocity{0,0};
      for(int i=0;i<hz*3;++i) {
        auto friction=[&](float v) { return std::copysign(std::max(0.f,std::abs(v)-.1f*dt*60.f),v); };
        velocity={friction(velocity.right),friction(velocity.forward)};
        const auto force=PortMouse::WalkForce(direction,velocity,fraction,maximum,mass,dt,acceleration);
        Check(std::hypot(force.right,force.forward)<=acceleration+.001f,"walking acceleration bounded");
        velocity.right+=force.right*dt/mass;velocity.forward+=force.forward*dt/mass;
      }
      Check(std::abs(std::hypot(velocity.right,velocity.forward)-fraction*maximum)<.0001f,
            "walking reaches genuinely low ground speed despite friction");
    }
  // Exercise both sides of the walk/native handoff, including near-100%.
  for(int hz:{30,60,120,240}) for(float divisor:{.7f,1.f}) for(auto dir:{PortMouse::Planar{1,0},PortMouse::Planar{0,-1},PortMouse::Planar{1,1}}) {
    const float dt=1.f/hz, mass=90, maximum=12, friction=.1f, acceleration=1000;
    dir=PortMouse::KeyboardRunInput(dir,divisor);
    auto v=PortMouse::GroundRunVelocity(dir,maximum,friction,mass,dt,acceleration);
    const float nativeSpeed=std::hypot(v.right,v.forward);
    KeyboardWalk ramp;ramp.Update(true,true,false,.25,1,dt);
    for(bool held:{true,false}) {
      float previous=std::hypot(v.right,v.forward);
      for(int i=0;i<4*hz;++i) {
        const float scale=float(ramp.Update(true,true,held,.25,1,dt));
        const auto damp=[&](float a){return std::copysign(std::max(0.f,std::abs(a)-friction*60*dt),a);};
        v={damp(v.right),damp(v.forward)};
        PortMouse::Planar force;
        if(scale<.9999f) force=PortMouse::KeyboardGroundForce(dir,v,scale,maximum,friction,mass,dt,acceleration);
        else force=PortMouse::ClampPlanar({PortMouse::AxisForce(dir.right,v.right,maximum,friction,mass,dt,acceleration,dt*60.f),
                                          PortMouse::AxisForce(dir.forward,v.forward,maximum,friction,mass,dt,acceleration,dt*60.f)},acceleration);
        v.right+=force.right*dt/mass;v.forward+=force.forward*dt/mass;
        const float speed=std::hypot(v.right,v.forward);
        Check(speed<=nativeSpeed+1e-4f,"transition never exceeds native run speed");
        Check(held?speed<=previous+1e-4f:speed>=previous-1e-4f,"no reversed acceleration during transition");
        previous=speed;
      }
      Check(std::abs(previous-nativeSpeed*(held?.25f:1.f))<.002f,"same running speed before and after walk");
    }
  }
  KeyboardWalk wide;wide.Update(true,true,false,.25,1,.01);
  for(int i=0;i<30;++i)wide.Update(true,true,true,.25,1,.01);
  Check(wide.value>.55 && wide.value<.65,"deliberately broad transition at 300 ms");
  for(bool alwaysRun:{false,true}) for(float fraction:{.25f,.5f,.7f}) for(int hz:{30,60,120,240}) {
    const float dt=1.f/hz;
    const auto direction=PortMouse::KeyboardRunInput({1,1},.7f);
    const auto run=PortMouse::GroundRunVelocity(direction,12,.1f,90,dt,1000);
    PortMouse::Planar velocity{run.right*fraction,run.forward*fraction};
    KeyboardWalk ramp;
    const bool held=alwaysRun;
    for(int i=0;i<hz*4;++i)ramp.Update(true,alwaysRun,held,.25,1,dt);
    Check(std::abs(ramp.value-.25)<1e-5,"walking selected in either Always run mode");
    // Ground -> jump (no friction) -> falling (air friction) -> ground.
    // Horizontal target uses the cached takeoff profile throughout.
    for(int phase=0;phase<4;++phase)for(int i=0;i<hz;++i) {
      const float friction=phase==1?0.f:(phase==2?.02f:.1f);
      const auto damp=[&](float v){return std::copysign(std::max(0.f,std::abs(v)-friction*dt*60),v);};
      velocity={damp(velocity.right),damp(velocity.forward)};
      auto force=PortMouse::KeyboardGroundForce(direction,velocity,fraction,12,.1f,90,dt,1000);
      velocity.right+=force.right*dt/90;velocity.forward+=force.forward*dt/90;
      Check(std::abs(velocity.right-run.right*fraction)<1e-5 &&
            std::abs(velocity.forward-run.forward*fraction)<1e-5,"no speed kick at takeoff, ledge or landing");
    }
  }
  for(auto walk:{0.,1.})for(auto run:{0.,2.}) {
    KeyboardWalk bounded;
    for(int i=0;i<400;++i)bounded.Update(true,false,false,walk,run,.01);
    Check(std::abs(bounded.value-(walk==0?.25:.5))<1e-6,"walk limits 25 to 50");
    for(int i=0;i<400;++i)bounded.Update(true,false,true,walk,run,.01);
    Check(std::abs(bounded.value-(run==0?.7:1.))<1e-6,"run limits 70 to 100");
  }
  // The engine's SDL -> PAD Y conversion can encode neutral as -1 before
  // PADClamp. Walk must not magnify anything the native deadzone would discard.
  for(int raw=-15;raw<=15;++raw) for(double scale:{.25,.5,.7,.999,1.}) {
    PADStatus native[4]{},walk[4]{};
    native[0].stickY=raw;
    walk[0].stickY=KeyboardWalkAxis(raw,false,scale);
    PADClamp(native);PADClamp(walk);
    Check(native[0].stickY==0 && walk[0].stickY==0,"neutral PAD values stay neutral with Walk");
  }
  Check(KeyboardWalkAxis(0,false,.5)==0,"no drift without movement key");
  Check(KeyboardWalkAxis(127,false,0)==0,"zero scale is stationary");
  Check(KeyboardWalkAxis(127,false,std::numeric_limits<double>::quiet_NaN())==127,"invalid scale safe");
  std::puts("Walk/Run transition, modes, speed, deadzone and diagonal tests passed");
}
