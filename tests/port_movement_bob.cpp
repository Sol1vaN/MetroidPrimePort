#include "port_movement_bob.h"
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <limits>
using namespace PortMovementBob;
void Check(bool ok, const char* message) {
  if (!ok) { std::fprintf(stderr, "Movement bob: %s\n", message); std::abort(); }
}
int main() {
  for (float fraction : {.01f, .05f, .15f, .35f, 1.f}) {
    float speed = fraction * 12.f;
    Check(std::abs(SpeedFraction(speed, 0, 12) - fraction) < 1e-6f, "strafe follows speed");
    Check(std::abs(SpeedFraction(0, -speed, 12) - fraction) < 1e-6f, "backward follows speed");
    Check(std::abs(SpeedFraction(speed / std::sqrt(2.f), speed / std::sqrt(2.f), 12) - fraction) < 1e-6f,
          "diagonal matches cardinal speed");
    Check(Cadence(fraction) >= fraction && Cadence(fraction) <= 1.f, "bounded low-speed cadence");
  }
  Check(Cadence(.01f) < .11f && Cadence(.35f) < Cadence(1.f), "creeping has no fixed cadence floor");
  Check(SpeedFraction(0, 0, 12) == 0 && SpeedFraction(12, 12, 12) == 1, "idle and overspeed");
  Check(SpeedFraction(1, 0, 0) == 0, "invalid maximum");
  float referencePhase = 0;
  for (int hz : {30, 60, 120, 240}) {
    float amplitude = 1, cadence = 1, phase = 0;
    const float dt = 1.f / hz;
    for (int i = 0; i < 3 * hz; ++i) {
      const float oldAmplitude = amplitude, oldCadence = cadence, oldPhase = phase;
      Advance(amplitude, cadence, phase, .01f, Cadence(.01f), .1f, dt);
      Check(amplitude <= oldAmplitude && amplitude >= .01f, "smooth bounded intensity decay");
      Check(cadence <= oldCadence && cadence >= .1f, "smooth bounded cadence decay");
      Check(phase > oldPhase && phase - oldPhase <= dt + 1e-6f, "continuous forward stride phase");
    }
    Check(std::abs(amplitude - .01f) < 1e-5f && std::abs(cadence - .1f) < 1e-5f, "creeping settles");
    if (!referencePhase) referencePhase = phase;
    Check(std::abs(phase - referencePhase) < 1e-5f, "phase consistent across tick rates");
    for (int i = 0; i < 3 * hz; ++i) Advance(amplitude, cadence, phase, 1, 1, .1f, dt);
    Check(std::abs(amplitude - 1) < 1e-5f && std::abs(cadence - 1) < 1e-5f, "running restores full bob");
    for (int i = 0; i < 3 * hz; ++i) Advance(amplitude, cadence, phase, 0, 0, .1f, dt);
    Check(amplitude < 1e-5f && cadence < 1e-5f, "stop fades rather than resetting phase");
    float saved = phase;
    Advance(amplitude, cadence, phase, 1, 1, .1f, 0);
    Advance(amplitude, cadence, phase, 1, 1, .1f, std::numeric_limits<float>::quiet_NaN());
    Check(phase == saved, "pause and invalid dt do not advance");
  }
  std::puts("Movement bob speed, direction, transitions and cadence tests passed");
}
