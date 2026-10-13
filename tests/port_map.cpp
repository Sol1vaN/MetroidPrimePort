#include "port_map_math.h"
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <limits>
using namespace PortMapMath;
void Check(bool ok,const char* why) {if(!ok){std::fprintf(stderr,"Map: %s\n",why);std::abort();}}
int main() {
  for(int hz:{30,60,120,144,240}) {
    double remaining=.12,total=0;
    for(int i=0;i<hz;++i) { double step=Drain(remaining,1.0/hz,14); Check(step>=0,"monotonic zoom");total+=step; }
    Check(std::abs(total-.12*(1-std::exp(-14)))<1e-8,"wheel travel independent of frame rate");
    Check(std::abs(total+remaining-.12)<1e-12,"no wheel input is lost");
    double x=0,out=0;
    for(int i=0;i<hz;++i) { x+=.4/hz;out+=Drain(x,1.0/hz); }
    for(int i=0;i<hz;++i)out+=Drain(x,1.0/hz);
    Check(std::abs(out-.4)<1e-8,"continuous mouse displacement preserved");
  }
  double p=.12;auto first=Drain(p,1.0/60,14);Check(first>0 && first<.12,"wheel does not jump immediately");
  p=-.12;Check(Drain(p,1.0/60)<0,"wheel down zooms out");
  p=.12;Check(Drain(p,0)==0 && p==.12,"paused ticks retain input");
  Check(Drain(p,1)==0 && p==0,"discard stale input after a stall");
  p=std::numeric_limits<double>::quiet_NaN();Check(Drain(p,.01)==0 && p==0,"reject invalid input");
  Check(Exit(true,true,false) && !Exit(true,false,true),"mouse map exits only with Z");
  Check(Exit(false,true,false) && Exit(false,false,true),"classic map exits with Z or B");
  const auto mouse=Move(true,1,.5f,-.25f,-1),classic=Move(false,1,.5f,-.25f,-1);
  Check(mouse.x==-1 && mouse.z==-.5f,"mouse mode pans with left stick only");
  Check(classic.x==.25f && classic.z==1,"classic mode pans with C-stick only");
  const auto invertedMouse=Move(true,1,.5f,-.25f,-1,true);
  const auto invertedClassic=Move(false,1,.5f,-.25f,-1,true);
  Check(invertedMouse.x==-mouse.x && invertedMouse.z==-mouse.z,"invert both mouse-mode pan axes");
  Check(invertedClassic.x==-classic.x && invertedClassic.z==-classic.z,"invert both classic-mode pan axes");
  Check(Move(true,0,0,1,1,true).x==0 && Move(true,0,0,1,1,true).z==0,"unused stick cannot pan in mouse mode");
  Check(Move(false,1,1,0,0,true).x==0 && Move(false,1,1,0,0,true).z==0,"unused stick cannot pan in classic mode");
  std::puts("Map controls and smoothing tests passed");
}
