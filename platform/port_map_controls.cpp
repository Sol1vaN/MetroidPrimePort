#include "port_map_controls.h"
#include "port_map_math.h"
#include "port_debug.h"
#include <dolphin/pad.h>
#include <SDL3/SDL_keyboard.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

namespace PortMapControls {
namespace {
std::mutex inputMutex;
double pendingX=0, pendingY=0, pendingZoom=0;
std::atomic<float> zoomSensitivity{100.f};
std::chrono::steady_clock::time_point lastMotion{}, lastWheel{};
PortMapMath::SoundGate zoomSound;
bool MouseActive() {
  return PortDebug::MouseAim() && PortDebug::MapScreenOpen() &&
         PortDebug::MouseCaptured() && !PortDebug::Visible();
}
}
void ResetInput() { std::lock_guard lock(inputMutex); pendingX=pendingY=pendingZoom=0; lastMotion=lastWheel={}; zoomSound={}; }
void AddMotion(float x,float y) {
  if (!MouseActive() || !std::isfinite(x) || !std::isfinite(y))return;
  const double sensitivity=PortDebug::MouseSensitivity();
  std::lock_guard lock(inputMutex);
  if (x!=0 || y!=0) lastMotion=std::chrono::steady_clock::now();
  pendingX=std::clamp(pendingX+x*sensitivity,-3.0,3.0);
  pendingY=std::clamp(pendingY+y*sensitivity,-3.0,3.0);
}
void AddWheel(float y) {
  if (!MouseActive() || !std::isfinite(y))return;
  std::lock_guard lock(inputMutex);
  if (y!=0) lastWheel=std::chrono::steady_clock::now();
  pendingZoom=std::clamp(pendingZoom+y*.12*ZoomSensitivity()/100.0,-2.0,2.0);
}
void TakeMotion(float dt,float& yaw,float& pitch) {
  std::lock_guard lock(inputMutex);
  yaw=float(PortMapMath::Drain(pendingX,dt));
  pitch=float(PortMapMath::Drain(pendingY,dt));
}
float TakeZoom(float dt) {
  std::lock_guard lock(inputMutex);
  return float(std::exp(PortMapMath::Drain(pendingZoom,dt,14.0)));
}
float ZoomSensitivity() { return zoomSensitivity.load(); }
void SetZoomSensitivity(float percent) { if(std::isfinite(percent))zoomSensitivity=std::clamp(percent,10.f,300.f); }
float RotationTravel(float yaw,float pitch,float dt) {
  std::lock_guard lock(inputMutex);
  const bool recent=std::chrono::steady_clock::now()-lastMotion<std::chrono::milliseconds(90);
  return PortMapMath::RotationTeeth(yaw,pitch,dt,recent);
}
bool ZoomSound(float before,float after,float dt) {
  std::lock_guard lock(inputMutex);
  const bool recent=std::chrono::steady_clock::now()-lastWheel<std::chrono::milliseconds(120);
  const float delta=before>0 && after>0?std::abs(std::log(after/before)):0;
  return zoomSound.Update(delta,dt,recent,.08f,.025f);
}
}
