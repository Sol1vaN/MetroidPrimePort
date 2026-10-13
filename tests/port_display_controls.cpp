#include "port_display_controls.h"
#include "port_mouse.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
void Check(bool ok) { if (!ok) std::abort(); }
int main() {
  using namespace PortDisplayControls;
  Check(BrightnessGain(0.f)==0.625f && BrightnessGain(50.f)==1.f && BrightnessGain(100.f)==1.375f);
  Check(BrightnessGain(50.01f)>BrightnessGain(50.f));
  Check(BrightnessGain(-1.f)==BrightnessGain(0.f));
  Check(BrightnessGain(std::numeric_limits<float>::quiet_NaN())==1.f);
  float at60=50.f, at120=50.f;
  for(int i=0;i<30;++i) at60=MoveSlider(at60,1,1.f/60,0,100);
  for(int i=0;i<60;++i) at120=MoveSlider(at120,1,1.f/120,0,100);
  Check(std::fabs(at60-at120)<0.001f && std::fabs(at60-70.f)<0.001f);
  Check(MoveSlider(50.123f,0,1.f/60,0,100)==50.123f);
  Check(MoveSlider(100.f,1,1.f/60,0,100)==100.f);
  Check(MoveSlider(0.f,-1,1.f/60,0,100)==0.f);
  PortMouse::AimState aim;
  Check(aim.Update(true,false,0,1,0,100,-100,0.00005f,false,false));
  Check(std::fabs(aim.yaw+0.005f)<1.e-7f && std::fabs(aim.pitch-0.005f)<1.e-7f);
  char value[32]; std::snprintf(value,sizeof(value),"%.6f",0.00005f);
  Check(std::fabs(std::strtof(value,nullptr)-0.00005f)<1.e-10f);
  Check(kSensitivityMin<=0.00005f);
  std::puts("Display controls and high-DPI sensitivity passed");
}
