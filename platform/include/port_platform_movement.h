#pragma once
// Retail platforms carry riders by transforming them in PreThink. Keep their
// jump/contact producer paired with retail movement for the whole flight;
// switching mover contracts halfway through a jump changes its contact state.
namespace PortPlatformMovement {
struct Mode {
  bool retail = false;
  void Update(bool enabled, bool grounded, bool jumping, bool descending, bool support) {
    if (!enabled) retail = false;
    else if (grounded) retail = support;
    else if (!jumping && descending && support) retail = true;
  }
};
}
