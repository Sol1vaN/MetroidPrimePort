#include "port_platform_movement.h"
#include <cstdio>
#include <initializer_list>
#include <cstdlib>
void Check(bool ok, const char* msg) { if (!ok) { std::fprintf(stderr,"%s\n",msg); std::abort(); } }
int main() {
  using PortPlatformMovement::Mode;
  for(bool goingUp:{false,true}) {
    Mode m;
    m.Update(true,true,false,!goingUp,true);
    Check(m.retail,"moving support selects paired retail jump/contact logic");
    // A full flight must use the same contract even after leaving probe range.
    for(int tick=0;tick<180;++tick) {
      m.Update(true,false,tick<20,tick>=20,false);
      Check(m.retail,"no mover switch midway through platform jump");
    }
    m.Update(true,true,false,true,false);
    Check(!m.retail,"ordinary terrain returns to official Remastered physics");
  }
  Mode loaded;
  loaded.Update(true,false,false,true,true);
  Check(loaded.retail,"recover a quickload already hovering above dynamic support");
  Mode passing;
  passing.Update(true,false,true,false,true);
  Check(!passing.retail,"rising past a platform is not grounded or captured");
  Mode ordinary;
  for(int tick=0;tick<100;++tick) ordinary.Update(true,false,tick<20,tick>20,false);
  Check(!ordinary.retail,"static terrain air movement remains official");
  loaded.Update(false,true,false,true,true);
  Check(!loaded.retail,"classic controls and morph ball do not opt into compatibility mode");
  std::puts("Platform movement mode: takeoff, full flight, landing and quickload recovery passed");
}
