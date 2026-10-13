#include "port_feedback.h"
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
using namespace PortFeedback;
void Check(bool ok,const char* why) { if(!ok){std::fprintf(stderr,"Feedback: %s\n",why);std::abort();} }
int main() {
  Quat reference;
  for(int hz:{30,60,120,144,240}) {
    const double dt=1.0/hz,rate=.2;
    // Constant angular speed must generate the same lag at every tick rate.
    auto target=LagTarget(std::sin(rate*dt),std::cos(rate*dt),0,dt,false);
    Check(std::abs(target.z+std::sin(rate/60))<1e-10,"lag target independent of tick size");
    Quat q;
    for(int i=0;i<hz;++i)q=LagStep(q,target,1,dt);
    if(hz==30)reference=q;
    Check(std::abs(q.z-reference.z)<1e-8,"lag tracking rate independent of tick size");
    for(int i=0;i<hz;++i)q=LagStep(q,{},1,dt);
    Check(std::abs(q.z)<1e-8,"lag smoothly returns to rest");
    float phase=0;int steps=0;
    for(int i=0;i<hz*30;++i)steps+=Footstep(phase,float(dt),.375f);
    Check(steps==80,"running footsteps preserve timing remainder");
    phase=0;steps=0;
    for(int i=0;i<hz*30;++i)steps+=Footstep(phase,float(dt),3.75f);
    Check(steps==8,"creeping footsteps follow slow movement");
  }
  float phase=0;int steps=0;
  for(int i=0;i<6000;++i) {
    float dt=i%2?.003f:.007f;
    steps+=Footstep(phase,dt,.375f);
  }
  Check(steps==80,"adaptive variable ticks retain cadence");
  phase=.7f;Check(!Footstep(phase,1,.375f)&&phase==0,"stalls never burst queued steps");
  phase=.7f;Check(!Footstep(phase,0,.375f)&&phase==.7f,"paused ticks do not emit");
  const auto tiny=LagTarget(.000001,1,0,1.0/240,false);
  Check(tiny.z<0 && std::abs(tiny.z)>1e-6,"tiny mouse motion does not quantize to zero");
  const auto inverted=LagTarget(.000001,1,0,1.0/240,true);
  Check(std::abs(tiny.z+inverted.z)<1e-10,"inverse HUD layer stays coherent");
  const auto middle=Blend({},tiny,.5);
  Check(middle.z<0 && middle.z>tiny.z,"presentation blends even tiny rotations");
  const auto neg=Blend(tiny,{-tiny.w,-tiny.x,-tiny.y,-tiny.z},.5);
  Check(std::abs(neg.z-tiny.z)<1e-10,"shortest quaternion path");
  Quat noisy;
  double lo=1,hi=-1;
  for(int i=0;i<300;++i) {
    const double angle=i%2?.001:0;
    noisy=LagStep(noisy,{std::cos(angle),0,0,std::sin(angle)},1,1.0/60);
    if(i>200) { lo=std::min(lo,noisy.z);hi=std::max(hi,noisy.z); }
  }
  Check(hi-lo<.00013,"fixed-tick packet noise is attenuated by at least 87 percent");
  Check(lo>0 && hi<.001,"smoothing cannot overshoot or flip direction");
  std::puts("HUD lag and footsteps timing tests passed");
}
