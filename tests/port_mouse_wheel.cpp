#include "../extern/aurora/lib/mouse_wheel.hpp"
#include <cstdlib>
#include <cstdio>
#include <limits>
void Check(bool ok) { if (!ok) std::abort(); }
int main() {
  aurora::input::MouseWheelPulses wheel;
  Check(wheel.Poll() == 0);
  wheel.Add(1.f); wheel.Add(-1.f); wheel.Add(2.f);
  for (int expected : {1,0,-1,0,1,0,1,0,0}) Check(wheel.Poll() == expected);
  wheel.Add(0.25f); Check(wheel.Poll() == 0);
  wheel.Add(0.75f); Check(wheel.Poll() == 1); Check(wheel.Poll() == 0);
  wheel.Add(0.5f); wheel.Add(-1.f); Check(wheel.Poll() == -1);
  wheel.Clear(); Check(wheel.Held() == 0); Check(wheel.Poll() == 0);
  wheel.Add(2.f); wheel.Clear(); Check(wheel.Poll() == 0);
  wheel.Add(std::numeric_limits<float>::infinity());
  wheel.Add(std::numeric_limits<float>::quiet_NaN()); Check(wheel.Poll() == 0);
  wheel.Add(1e30f);
  for (int i=0;i<32;++i) { Check(wheel.Poll()==1); Check(wheel.Poll()==0); }
  Check(wheel.Poll()==0);
  std::puts("Wheel pulse tests passed");
}
