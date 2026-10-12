// Exercise real map input and the native glyph upload without a game session.
#include "port_map_controls.h"
#include "port_map_math.h"
#include "port_map_glyph.h"
#include <SDL3/SDL.h>
#include <chrono>
#include <thread>
#include <fstream>
#include <iterator>
#include <cstdio>
#include <cstdlib>
bool mouse=true,visible=false,active=true,captured=true;
namespace PortDebug {
bool MouseAim(){return mouse;} bool Visible(){return visible;}
bool MapScreenOpen(){return active;} bool MouseCaptured(){return captured;}
float MouseSensitivity(){return .00005f;}
}
void Check(bool ok,const char* why){if(!ok){std::fprintf(stderr,"Map UI: %s\n",why);std::abort();}}
PortMapGlyph::Image Load(const char* name) {
 std::ifstream in(std::string(PORT_TEST_ASSETS)+"/"+name+".dds",std::ios::binary);
 std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)),{});
 PortMapGlyph::Image out;Check(PortMapGlyph::Decode(bytes,out),"decode original binding DDS");return out;
}
int Reds(const PortMapGlyph::Image& a){int count=0;for(int p=0;p<4096;p+=4)count+=PortMapGlyph::Red(a,p);return count;}
int main() {
 using namespace PortMapControls;
 ResetInput();AddMotion(100,-100);float x,y;TakeMotion(1.f/60,x,y);
 Check(x>0 && y<0 && x<.005f,"real mouse input is smoothly rotated");
 Check(RotationTravel(x,y,1.f/60)>0,"deliberate rotation sounds");
 std::this_thread::sleep_for(std::chrono::milliseconds(110));
 TakeMotion(1.f/60,x,y);Check(x>0 && RotationTravel(x,y,1.f/60)==0,"visual tail continues silently after input stops");
 double total=x;for(int i=0;i<240;++i){TakeMotion(1.f/60,x,y);total+=x;}
 Check(total>0 && total<.005,"input decay remains smooth");
 ResetInput();AddMotion(1,1);TakeMotion(1.f/60,x,y);
 Check(x>0 && RotationTravel(x,y,1.f/60)<1,"micromovement alone cannot cross a whole tooth");
 ResetInput();AddWheel(1);float zoom=TakeZoom(1.f/60);Check(zoom>1,"wheel up zooms in");
 Check(ZoomSound(10,10/zoom,1.f/60),"wheel zoom sounds");
 std::this_thread::sleep_for(std::chrono::milliseconds(140));zoom=TakeZoom(1.f/60);
 Check(zoom>1 && !ZoomSound(10,10/zoom,1.f/60),"zoom tail stays silent");
 ResetInput();AddWheel(-1);Check(TakeZoom(1.f/60)<1,"wheel down zooms out");
 ResetInput();visible=true;AddWheel(1);AddMotion(10,10);TakeMotion(.01f,x,y);
 Check(x==0 && y==0 && TakeZoom(.01f)==1,"overlay blocks map input");visible=false;
 mouse=false;AddMotion(10,10);AddWheel(1);TakeMotion(.01f,x,y);
 Check(x==0 && y==0 && TakeZoom(.01f)==1,"classic mode ignores raw mouse input");
 SetZoomSensitivity(0);Check(ZoomSensitivity()==10,"minimum zoom sensitivity");
 SetZoomSensitivity(500);Check(ZoomSensitivity()==300,"maximum zoom sensitivity");SetZoomSensitivity(100);
 for(float dt:{1.f/30,1.f/60,1.f/144,1.f/240}) {
  PortMapMath::SoundGate gate;
  Check(!gate.Update(.02f*dt,dt,true,.035f,.015f),"start threshold is frame-rate independent");
  Check(gate.Update(.04f*dt,dt,true,.035f,.015f),"motion opens sound gate");
  Check(gate.Update(.02f*dt,dt,true,.035f,.015f),"hysteresis prevents chatter");
  Check(!gate.Update(.01f*dt,dt,true,.035f,.015f),"slow tail closes sound gate");
  Check(!gate.Update(1,dt,false,.035f,.015f),"stale input always silent");
 }
 // Equal angular travel produces equal teeth, independent of time and FPS.
 for(float dt:{1.f/30,1.f/60,1.f/144,1.f/240}) {
  for(float speed:{.0005f,.005f,.02f}) {
   float total=0;
   for(int i=0;i<int(std::round(4/dt));++i)
    total+=PortMapMath::RotationTeeth(speed*dt,0,dt,true);
   Check(std::abs(total-speed/.05f*MUSYX_MAP_STEP_NATIVE_HZ*4)<.006f,"tooth budget follows angular distance including very slow motion");
  }
  Check(PortMapMath::RotationTeeth(2,0,dt,true)<=float(MUSYX_MAP_STEP_MAX_HZ*dt)+1e-6f,"flicks have an audio-only upper bound");
  Check(PortMapMath::RotationTeeth(.001f,0,dt,false)==0,"stale motion tail cannot add teeth");
  Check(PortMapMath::RotationTeeth(0,0,dt,true)==0,"no movement means no tooth");
 }
 // More mouse speed remains below the native rhythm, while the old very-slow
 // spacing and native PCM fragments are unchanged.
 for(float speed:{.05f,.08f,.10f})
  Check(PortMapMath::RotationTeeth(speed/60,0,1.f/60,true)*60<MUSYX_MAP_STEP_NATIVE_HZ,
        "extended slow range does not prematurely reach native cadence");
 const float nativeSpeed=float(.02+.015*std::expm1(2.0));
 Check(std::abs(PortMapMath::RotationTeeth(nativeSpeed/60,0,1.f/60,true)*60-MUSYX_MAP_STEP_NATIVE_HZ)<1e-4,
       "extended response reaches the original rhythm continuously");
 // Smooth monotonic cadence up to the 130% ceiling, without a hard knee at 100%.
 float previous=0;
 for(int i=0;i<2000;++i) {
  float rate=PortMapMath::RotationTeeth(i*.001f/60,0,1.f/60,true)*60;
  Check(rate>=previous && rate<=MUSYX_MAP_STEP_MAX_HZ+1e-5,"continuous bounded rate response");
  previous=rate;
 }
 using namespace PortMapGlyph;
 auto left=Load("mouse_left"),right=Load("mouse_right"),wheel=Load("mouse_scroll"),neutral=Neutral(left);
 auto up=HalfWheel(wheel,true),down=HalfWheel(wheel,false);
 Check(Reds(left)>0 && Reds(right)>0 && Reds(neutral)==0,"original red mouse buttons retained");
 Check(Reds(up)>0 && Reds(down)>0 && Reds(up)+Reds(down)==Reds(wheel),"wheel halves preserve original artwork");
 Check(Fade(0)==0 && Fade(1.1f)==1 && Fade(2.2f)==0 && std::abs(Fade(.975f)-.5f)<.001,"smooth alternating prompt timing");
 auto first=Blend(left,right,0),second=Blend(left,right,1);
 for(int p=0;p<4096;p+=4)if(left.rgba[p+3])for(int c=0;c<4;++c)Check(std::abs(first.rgba[p+c]-left.rgba[p+c])<=1,"first glyph endpoint exact");
 for(int p=0;p<4096;p+=4)if(right.rgba[p+3])for(int c=0;c<4;++c)Check(std::abs(second.rgba[p+c]-right.rgba[p+c])<=1,"second glyph endpoint exact");
 auto tiled=NativeTiles(left,32);
 for(int y=0;y<32;++y)for(int x=0;x<32;++x) {
  int block=((y/4)*8+x/4)*64,off=((y%4)*4+x%4)*2,p=((31-y)*32+x)*4;
  Check(tiled[block+off]==left.rgba[p+3],"GX alpha layout and vertical orientation");
  if(left.rgba[p+3])Check(std::abs(tiled[block+off+1]-left.rgba[p])<=1 && std::abs(tiled[block+32+off]-left.rgba[p+1])<=1 && std::abs(tiled[block+33+off]-left.rgba[p+2])<=1,"GX color channels round trip");
 }
 Check(NativeTiles(left,24).size()==24*24*4,"compact native glyph tiles");
 auto menu=Load("keyboard_menu");Check(menu.rgba!=Missing().rgba,"Menu has actual keycap artwork");
 auto off=RotationOffset(.123f),next=RotationOffset(.123f+1.f/60);
 Check(std::hypot(off[0]-next[0],off[1]-next[1])<.8f,"mouse gesture advances continuously");
 Check(std::abs(RotationOffset(.75f)[0]-3.f)<.001,"mouse gesture has visible amplitude");
 auto alpha=[](const Image& img){double a=0;for(int p=3;p<4096;p+=4)a+=img.rgba[p];return a;};
 for(int i=0;i<180;++i) {
  auto pos=RotationOffset(i/60.f);auto moved=Transform(neutral,1.12f,pos[0],pos[1]);
  Check(alpha(moved)>alpha(neutral)*1.2,"animated mouse silhouette is not clipped");
 }
 std::vector<Image> examples{Transform(neutral,1.12f),Transform(left,1.22f),Blend(left,right,.5),Transform(right,1.22f),up,down,menu};
 const int scale=4,w=int(examples.size())*32*scale,h=32*scale;std::vector<uint8_t> pixels(w*h*4,255);
 for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
  const auto& img=examples[x/(32*scale)];int src=((y/scale)*32+(x/scale)%32)*4,dst=(y*w+x)*4;
  float alpha=img.rgba[src+3]/255.f;for(int c=0;c<3;++c)pixels[dst+c]=uint8_t(img.rgba[src+c]*alpha+15*(1-alpha));
 }
 auto* surface=SDL_CreateSurfaceFrom(w,h,SDL_PIXELFORMAT_RGBA32,pixels.data(),w*4);
 Check(surface && SDL_SaveBMP(surface,"map-native-glyphs.bmp"),"native artwork preview");SDL_DestroySurface(surface);
 std::puts("Map motion, sound gates and original native glyph upload passed");
}
