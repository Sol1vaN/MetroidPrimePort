#pragma once
namespace PortMapControls {
void ResetInput();
void AddMotion(float x, float y);
void AddWheel(float y);
void TakeMotion(float dt, float& yaw, float& pitch);
float TakeZoom(float dt);
float ZoomSensitivity();
void SetZoomSensitivity(float percent);
float RotationTravel(float yaw,float pitch,float dt);
bool ZoomSound(float before,float after,float dt);
}
