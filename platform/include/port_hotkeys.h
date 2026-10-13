#pragma once
#include <SDL3/SDL_scancode.h>
#include <array>

namespace PortHotkeys {
enum Action { None = -1, FrameLimit, SaveState, LoadState, Count };
// Keep the newly added photo action off pre-existing game and port bindings.
template <typename IsUsed>
int DefaultPhotoKey(IsUsed used) {
  for (const int key : {SDL_SCANCODE_F8, SDL_SCANCODE_F7, SDL_SCANCODE_F6,
                       SDL_SCANCODE_F4, SDL_SCANCODE_F3, SDL_SCANCODE_F2})
    if (!used(key)) return key;
  return -1;
}

inline bool ValidKey(int key) {
  return key == -1 || (key > SDL_SCANCODE_UNKNOWN && key < SDL_SCANCODE_COUNT &&
      key != SDL_SCANCODE_F1 && key != SDL_SCANCODE_F11 && key != SDL_SCANCODE_F12 &&
      key != SDL_SCANCODE_ESCAPE);
}
struct Bindings {
  std::array<int, Count> keys{SDL_SCANCODE_F10, SDL_SCANCODE_F5, SDL_SCANCODE_F9};
  Action capturing = None;
  bool Assign(Action action, int key) {
    if (action < FrameLimit || action >= Count || !ValidKey(key)) return false;
    // Assigning a key transfers it from any other port action.
    for (int& other : keys) if (key >= 0 && other == key) other = -1;
    keys[action] = key;
    return true;
  }
  Action Press(int key, bool repeat, bool blocked, bool saveEnabled, bool& changed) {
    changed = false;
    if (repeat) return None;
    if (capturing != None) {
      if (key == SDL_SCANCODE_ESCAPE) { capturing = None; return None; }
      if (key == SDL_SCANCODE_DELETE || key == SDL_SCANCODE_BACKSPACE) key = -1;
      if (Assign(capturing, key)) { capturing = None; changed = true; }
      return None; // Capturing never triggers the newly assigned action.
    }
    if (blocked || key <= SDL_SCANCODE_UNKNOWN) return None;
    for (int i = FrameLimit; i < Count; ++i) {
      if (key == keys[i] && (i == FrameLimit || saveEnabled)) return static_cast<Action>(i);
    }
    return None;
  }
};
} // namespace PortHotkeys
