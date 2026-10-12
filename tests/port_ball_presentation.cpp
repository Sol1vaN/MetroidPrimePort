#include "port_ball_presentation.h"
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <limits>
using namespace PortBallPresentation;
void Check(bool ok,const char* why) { if(!ok){std::fprintf(stderr,"Ball presentation: %s\n",why);std::abort();} }
bool Near(double a,double b) { return std::abs(a-b)<1e-9; }
Pose Rolling(double time, double speed, double radius) {
  const double angle=speed*time/radius;
  return {speed*time,0,radius,{std::cos(angle*.5),0,std::sin(angle*.5),0}};
}
int main() {
  // Analytic no-slip roll: center must travel on a straight horizontal line,
  // even when the sphere rolls more than the generic actor's 45-degree cutoff.
  for(double speed:{3.,12.,35.}) for(int hz:{60,120,144,240}) {
    History history;
    const double radius=.5;
    for(int frame=0;frame<hz*2;++frame) {
      const double sample=double(frame)/hz;
      const unsigned tick=unsigned(std::floor(sample*60+1e-9));
      const double t=sample*60-tick;
      const auto from=Rolling(tick/60.,speed,radius);
      const auto to=Rolling((tick+1)/60.,speed,radius);
      history.Snapshot(from,tick+1,true);
      Pose out;
      Check(history.Present(to,tick+1,true,t,out),"rolling remains interpolated at every render rate");
      Check(Near(out.x,speed*sample) && Near(out.y,0) && Near(out.z,radius),"ball center never orbits the player origin");
      const auto expected=Rolling(sample,speed,radius).rotation;
      const auto actual=out.rotation;
      const double dot=actual.w*expected.w+actual.x*expected.x+actual.y*expected.y+actual.z*expected.z;
      Check(std::abs(std::abs(dot)-1)<1e-8,"orientation tracks continuous rolling including fast turns");
    }
  }
  History history; Pose from{},to{1,0,.5,{}},out;
  Check(!history.Present(to,1,true,.5,out),"new balls cannot use stale transforms");
  history.Snapshot(from,1,false);
  Check(!history.Present(to,1,true,.5,out),"morph transition starts without old ball pose");
  history.Snapshot(from,2,true);
  Check(!history.Present(to,2,false,.5,out),"unmorph transition retains native presentation");
  Check(!history.Present(to,3,true,.5,out),"stale generation rejected");
  Check(!history.Present(to,2,true,-1,out),"disabled interpolation stays native");
  Check(!history.Present(to,2,true,1,out),"current tick endpoint stays native");
  Check(!history.Present(to,2,true,std::numeric_limits<double>::quiet_NaN(),out),"invalid phase rejected");
  Check(!history.Present({5,0,0,{}},2,true,.5,out),"teleports snap");
  Check(history.Present({0,0,0,{0,0,1,0}},2,true,.5,out),"large roll cannot disable positional smoothing");
  std::puts("Ball center, rolling, frame rates and transition tests passed");
}
