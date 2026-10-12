#pragma once
#include <array>
#include <cmath>
#include <cstddef>

namespace aurora::input {
// Retain detents across render frames until simulation consumes each tap.
// A release poll separates taps, including several in the same event batch.
class MouseWheelPulses {
  std::array<int, 32> pending{};
  std::size_t head = 0;
  std::size_t count = 0;
  float fraction = 0.f;
  int held = 0;
public:
  void Clear() { head = count = 0; fraction = 0.f; held = 0; }
  void Add(float delta) {
    if (!std::isfinite(delta) || delta == 0.f) return;
    if ((fraction > 0.f && delta < 0.f) || (fraction < 0.f && delta > 0.f)) fraction = 0.f;
    fraction += delta > 32.f ? 32.f : delta < -32.f ? -32.f : delta;
    while (std::fabs(fraction) >= 1.f) {
      const int direction = fraction > 0.f ? 1 : -1;
      if (count < pending.size()) pending[(head + count++) % pending.size()] = direction;
      fraction -= static_cast<float>(direction);
    }
  }
  int Poll() {
    if (held != 0) { held = 0; return 0; }
    if (count == 0) return 0;
    held = pending[head];
    head = (head + 1) % pending.size();
    --count;
    return held;
  }
  int Held() const { return held; }
};
} // namespace aurora::input
