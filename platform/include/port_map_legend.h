#pragma once
#include <string>
class CFontImageDef;
class CGuiFrame;
namespace PortMapLegend {
CFontImageDef Image(const wchar_t* spec, int length);
void Initialize(CGuiFrame&);
void Update(CGuiFrame& frame, bool mouse, bool worldAvailable);
}
