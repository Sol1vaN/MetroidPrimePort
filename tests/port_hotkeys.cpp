#include "port_hotkeys.h"
#include <cstdio>
#include <cstdlib>
using namespace PortHotkeys;
void Check(bool ok) { if (!ok) std::abort(); }
int main() {
  Bindings b;
  bool changed = false;
  Check(b.Press(SDL_SCANCODE_F5,false,false,true,changed)==SaveState);
  Check(b.Press(SDL_SCANCODE_F9,false,false,true,changed)==LoadState);
  b.Assign(SaveState,-1); b.Assign(LoadState,-1);
  Check(b.Press(SDL_SCANCODE_F5,false,false,true,changed)==None);
  Check(b.Press(SDL_SCANCODE_F9,false,false,true,changed)==None);
  b.capturing=FrameLimit;
  Check(b.Press(SDL_SCANCODE_F9,false,false,true,changed)==None && changed);
  Check(b.Press(SDL_SCANCODE_F9,false,false,true,changed)==FrameLimit);
  b.capturing=SaveState;
  Check(b.Press(SDL_SCANCODE_F9,false,false,true,changed)==None && changed);
  Check(b.keys[FrameLimit]==-1 && b.keys[SaveState]==SDL_SCANCODE_F9);
  Check(b.Press(SDL_SCANCODE_F9,true,false,true,changed)==None);
  Check(b.Press(SDL_SCANCODE_F9,false,true,true,changed)==None);
  Check(b.Press(SDL_SCANCODE_F9,false,false,false,changed)==None);
  Check(b.Press(SDL_SCANCODE_F9,false,false,true,changed)==SaveState);
  b.capturing=LoadState;
  Check(b.Press(SDL_SCANCODE_F1,false,false,true,changed)==None && !changed && b.capturing==LoadState);
  Check(b.Press(SDL_SCANCODE_ESCAPE,false,false,true,changed)==None && !changed && b.capturing==None);
  Check(b.keys[LoadState]==-1);
  b.capturing=SaveState;
  Check(b.Press(SDL_SCANCODE_DELETE,false,false,true,changed)==None && changed && b.keys[SaveState]==-1);
  Check(!b.Assign(LoadState,SDL_SCANCODE_F11) && !b.Assign(LoadState,SDL_SCANCODE_F12));
  Check(!b.Assign(LoadState,SDL_SCANCODE_COUNT) && !b.Assign(LoadState,0));
  Check(b.Press(-1,false,false,true,changed)==None);
  std::puts("Port hotkeys: unbinding, rebinding, conflicts, capture and input guards passed");
}
