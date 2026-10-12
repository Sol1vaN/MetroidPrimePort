#include "port_map_legend.h"
#include "port_input_devices.h"
#include "port_map_glyph.h"
#include "port_prompts.h"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include <dolphin/pad.h>
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <cwchar>
#include <cstring>

namespace PortMapLegend {
namespace {
constexpr int kRotate=-100,kZoom=-101;
int Binding(bool axis,unsigned code) {
 return PortInputDevices::KeyBindingForPad(axis,code);
}

bool LargerKey(int key,bool classic) {
 return key==SDL_SCANCODE_TAB || key==SDL_SCANCODE_LCTRL || key==SDL_SCANCODE_RCTRL ||
        (classic && (key==SDL_SCANCODE_SPACE || key==SDL_SCANCODE_LSHIFT || key==SDL_SCANCODE_RSHIFT));
}
std::wstring Prompt(int a,int b,int size=32,bool classic=false) {
 const int large=(LargerKey(a,classic)?1:0)|(LargerKey(b,classic)?2:0);
 return L"&image=PORT,"+std::to_wstring(a)+L","+std::to_wstring(b)+L","+std::to_wstring(size)+L","+std::to_wstring(large)+L";";
}
PortMapGlyph::Image Glyph(int key) {
 std::vector<uint8_t> bytes;PortMapGlyph::Image img;
 if(!PortPrompts::ReadKeyGlyph(key,bytes) || !PortMapGlyph::Decode(bytes,img))return PortMapGlyph::Missing();
 if(key==PAD_KEY_MOUSE_WHEEL_UP || key==PAD_KEY_MOUSE_WHEEL_DOWN)
  img=PortMapGlyph::HalfWheel(img,key==PAD_KEY_MOUSE_WHEEL_UP);
 return img;
}
void Set(CGuiFrame& frame,const char* name,const std::wstring& text) {
 auto* pane=static_cast<CGuiTextPane*>(frame.FindWidget(name));if(!pane)return;
 auto& support=pane->TextSupport();support.SetWordWrap(false);
 support.SetImageBaseline(std::strcmp(name,"textpane_left")!=0);
 if(std::strncmp(name,"textpane_instructions",21)==0) {
  // Enlarge native lettering within the existing, generously spaced columns.
  // Two icon rows remain inside the original frame; no panel asset changes.
  if(support.GetTextBoundingWidth()!=110)support.SetExtentX(110);
  if(support.GetTextBoundingHeight()!=58)support.SetExtentY(58);
 }
 // Use the original font, colours, alignment, pane transforms and frame model.
 support.SetText(rstl::wstring(text.c_str()));
}
}
CFontImageDef Image(const wchar_t* spec,int length) {
 int a=-1,b=-1,size=32,large=0;
 std::wstring text(spec,size_t(std::max(length,0)));
 if(std::swscanf(text.c_str(),L"%d,%d,%d,%d",&a,&b,&size,&large)<3) a=b=-1;
 const bool nativeButtonCrop=size==22;
 const bool zoomTop=size==41; // Fixed column width; keep the small icon height.
 size=(size==40 || zoomTop || large)?40:32;
 const bool rotate=a==kRotate,zoom=a==kZoom;
 auto first=rotate?PortMapGlyph::Neutral(Glyph(PAD_KEY_MOUSE_LEFT)):zoom?Glyph(PAD_KEY_MOUSE_WHEEL_UP):Glyph(a);
 auto second=zoom?Glyph(PAD_KEY_MOUSE_WHEEL_DOWN):Glyph(b);
 // Grow only the requested keycaps by 25%, using their original artwork.
 // Alternate non-target glyphs retain their old size within the larger canvas.
 if(!rotate) {
  first=PortMapGlyph::Transform(first,1.22f*((large || zoomTop) && !(large&1)?32.f/size:1.f));
  second=PortMapGlyph::Transform(second,1.22f*((large || zoomTop) && !(large&2)?32.f/size:1.f));
 }
 const float fps=PortMapGlyph::kAnimationFPS;
 const int count=rotate?180:(zoom||a!=b?132:1);
 rstl::vector<TToken<CTexture>> textures;textures.reserve(count);
 for(int i=0;i<count;++i) {
  PortMapGlyph::Image img=first;
  if(rotate) {
   const auto offset=PortMapGlyph::RotationOffset(i/fps);
   img=PortMapGlyph::Transform(first,1.12f,offset[0],offset[1]);
  } else {
   if(count>1)img=PortMapGlyph::Blend(first,second,PortMapGlyph::Fade(i/fps));
  }
  auto tiled=PortMapGlyph::NativeTiles(img,size);
  CTexture* texture=rs_new CTexture(kTF_RGBA8,short(size),short(size),1);
  std::memcpy(texture->Lock(),tiled.data(),tiled.size());texture->UnLock();
  textures.push_back(TToken<CTexture>(texture));
 }
 CFontImageDef image(textures,fps,CVector2f(1.f,nativeButtonCrop?.68f:zoomTop?.8f:1.f));
 // Center the native font's visible letter height on the icon, rather than
 // using the generic 5/6 image baseline which pushed labels downwards.
 // Zoom in has a 40-wide, 32-high slot: move only its artwork down five
 // logical pixels to match Rotate V, without shifting the label or clipping it.
 if(!nativeButtonCrop)image.PortSetAlignment(size/2+5,0,zoomTop?5:0);
 else if(a==SDL_SCANCODE_MENU || a==SDL_SCANCODE_APPLICATION)
   image.PortSetAlignment(image.CalculateBaseline(),-3);
 return image;
}
void Initialize(CGuiFrame& frame) {
 // Apply once to the native widgets; their frame and slide animations stay intact.
 for(const char* name:{"textpane_instructions","textpane_instructions1","textpane_instructions2"}) {
  if(auto* pane=frame.FindWidget(name)) {
   const auto position=pane->GetLocalPosition();
   pane->SetLocalPosition(position+CVector3f(0.f,0.f,.30f));
  }
 }
}
void Update(CGuiFrame& frame,bool mouse,bool worldAvailable) {
 auto axis=[](unsigned id){return Binding(true,id);};
 auto button=[](unsigned id){return Binding(false,id);};
 const int z=button(PAD_TRIGGER_Z),y=button(PAD_BUTTON_Y),a=button(PAD_BUTTON_A);
 Set(frame,"textpane_left",Prompt(z,mouse?z:button(PAD_BUTTON_B),22,!mouse)+L"\nExit");
 Set(frame,"textpane_yicon",Prompt(y,y,22,!mouse)+L"  Key/Legend");
 Set(frame,"textpane_right1",worldAvailable?Prompt(a,a,32,!mouse):L"");
 if(mouse) {
  Set(frame,"textpane_instructions",Prompt(kRotate,kRotate,40)+L" Rotate");
  Set(frame,"textpane_instructions2",Prompt(kZoom,kZoom,40)+L" Zoom");
 } else {
  Set(frame,"textpane_instructions",Prompt(axis(2),axis(3),32,true)+L" Rotate V\n"+Prompt(axis(1),axis(0),32,true)+L" Rotate H");
  Set(frame,"textpane_instructions2",L"&just=ncenter;"+Prompt(axis(9),axis(9),41,true)+L" Zoom in\n"+Prompt(axis(8),axis(8),32,true)+L" Zoom out");
 }
 const unsigned base=mouse?PAD_AXIS_LEFT_X_POS:PAD_AXIS_RIGHT_X_POS;
 Set(frame,"textpane_instructions1",Prompt(axis(base+2),axis(base+3),32,!mouse)+L" Move U/D\n"+Prompt(axis(base+1),axis(base),32,!mouse)+L" Move L/R");
}
}
