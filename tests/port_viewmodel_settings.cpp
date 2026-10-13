#include "port_viewmodel_settings.h"
#include <cstdio>
#include <cstdlib>
using namespace PortViewmodelSettings;
void Check(bool ok) { if(!ok) std::abort(); }
bool Near(float a,float b){return std::abs(a-b)<.00001f;}
int main() {
  const Position native{.7f,-.4f,.9f};
  for (bool original : {false,true}) for (bool visual : {false,true}) {
    const auto effective=EffectiveGunOffset(original,visual,native);
    Check(Near(effective.x,original||visual?0.f:native.x));
    Check(Near(effective.y,original||visual?0.f:native.y));
    Check(Near(effective.z,original||visual?0.f:native.z));
  }
  Settings s;Check(!s.enabled && Near(s.Fov(80),55));
  s.enabled=true;Check(Near(s.Fov(80),80));
  auto p=s.GetPosition(75);Check(Near(p.x,.25f)&&Near(p.y,.30f)&&Near(p.z,-.35f));
  p=s.GetPosition(100);Check(Near(p.x,.235f)&&Near(p.y,.14f));
  p=s.GetPosition(140);Check(Near(p.x,.22f)&&Near(p.y,-.02f));
  s.automatic=false;s.right=25;s.forward=20;s.up=-31;s.fov=70;
  p=s.GetPosition(110);Check(Near(p.x,.25f)&&Near(p.y,.20f)&&Near(p.z,-.31f)&&Near(s.Fov(110),70));
  s={};Check(!s.enabled&&s.automatic&&Near(s.fov,55)&&Near(s.right,25)&&Near(s.forward,30)&&Near(s.up,-35));
  for(float aspect:{4.f/3.f,16.f/9.f,21.f/9.f,32.f/9.f})for(float fov:{55.f,80.f,100.f}){
    const auto fit=VisorScale(aspect,fov);
    const float y=std::tan(fov*.00872664626f),retail=std::tan(55.f*.00872664626f);
    Check(Near(fit.x/(aspect*y),1.f/((4.f/3.f)*retail)));
    Check(Near(fit.z/y,1.f/retail)&&fit.y==1.f);
  }
  std::puts("Viewmodel: automatic/manual placement, reset, projection and visor coverage passed");
}
