#ifndef _CFONTIMAGEDEF
#define _CFONTIMAGEDEF

#include <Kyoto/Graphics/CTexture.hpp>
#include <Kyoto/Math/CVector2f.hpp>
#include <Kyoto/TToken.hpp>
#include <rstl/vector.hpp>

class CTexture;
class CVector2f;
class CFontImageDef {
public:
  CFontImageDef(const TToken< CTexture >& texture, const CVector2f& cropFactor);
  CFontImageDef(const rstl::vector< TToken< CTexture > >& texture, float fps,
                const CVector2f& cropFactor);

  bool IsLoaded() const;
  const rstl::vector< TToken< CTexture > >& GetImages() const { return mTextures; }
  float GetFps() const { return mFPS; }
  const CVector2f& GetScale() const { return mCropFactor; }
  int GetMonoWidth() const {
    TToken< CTexture > tex = mTextures[0];
    return tex->GetWidth() * mCropFactor.GetX();
  }
  int GetMonoHeight() const {
    TToken< CTexture > tex = mTextures[0];
    return tex->GetHeight() * mCropFactor.GetY();
  }
  int GetWidth() const { return GetMonoWidth(); }
  int GetHeight() const { return GetMonoHeight(); }
  int CalculateBaseline() const;
#ifdef TARGET_PC
  void PortSetAlignment(int baseline,int offsetX=0,int offsetY=0) {
    mPortBaseline=baseline;mPortOffsetX=offsetX;mPortOffsetY=offsetY;
  }
  int PortOffsetX() const { return mPortOffsetX; }
  int PortOffsetY() const { return mPortOffsetY; }
#endif
  int CalculateHeight() const;

private:
#ifdef TARGET_PC
  int mPortBaseline=-1;
  int mPortOffsetX=0;
  int mPortOffsetY=0;
#endif
  float mFPS;
  rstl::vector< TToken< CTexture > > mTextures;
  CVector2f mCropFactor;
};

CHECK_SIZEOF(CFontImageDef, 0x1c)

#endif // _CFONTIMAGEDEF
