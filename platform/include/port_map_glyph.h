#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
namespace PortMapGlyph {
struct Image { int size=32; std::vector<uint8_t> rgba=std::vector<uint8_t>(32*32*4); };
inline bool Decode(const std::vector<uint8_t>& d,Image& out) {
 auto u32=[&](size_t p){return uint32_t(d[p])|(uint32_t(d[p+1])<<8)|(uint32_t(d[p+2])<<16)|(uint32_t(d[p+3])<<24);};
 if(d.size()<128 || d[0]!='D'||d[1]!='D'||d[2]!='S'||d[3]!=' '||u32(12)!=32||u32(16)!=32||u32(84)!=0||u32(88)!=32||u32(92)!=255||d.size()<128+4096)return false;
 out=Image{};
 // Binding DDS files are bottom-up; work top-down until the native GX upload.
 for(int y=0;y<32;++y)std::copy_n(d.data()+128+(31-y)*128,128,out.rgba.data()+y*128);
 return true;
}
inline Image Missing() {
 Image out;
 // Only the missing-binding indicator is supplemental art (N/A, red).
 const char* letters[7]={"#   #    /    ### ","##  #   /     # # ","# # #   /     # # ","# # #  /      ### ","#  ##  /      # # ","#   # /       # # ","#   # /       # # "};
 for(int y=0;y<7;++y)for(int x=0;x<17;++x)if(letters[y][x]!=' ') {
  int p=((12+y)*32+7+x)*4;out.rgba[p]=255;out.rgba[p+1]=70;out.rgba[p+2]=75;out.rgba[p+3]=255;
 }
 return out;
}
inline bool Red(const Image& a,int p) {return a.rgba[p]>a.rgba[p+1]+30 && a.rgba[p]>a.rgba[p+2]+20 && a.rgba[p+3]>40;}
inline Image Neutral(Image a) {
 for(int p=0;p<int(a.rgba.size());p+=4)if(Red(a,p))a.rgba[p]=a.rgba[p+1]=a.rgba[p+2]=240;
 return a;
}
inline Image HalfWheel(Image a,bool upper) {
 int lo=32,hi=0;
 for(int y=0;y<32;++y)for(int x=0;x<32;++x)if(Red(a,(y*32+x)*4)){lo=std::min(lo,y);hi=std::max(hi,y);}
 const float middle=(lo+hi)*.5f;
 for(int y=0;y<32;++y)for(int x=0;x<32;++x) {
  int p=(y*32+x)*4;if(Red(a,p) && ((y>middle)==upper))a.rgba[p]=a.rgba[p+1]=a.rgba[p+2]=240;
 }
 return a;
}
inline Image Blend(const Image& a,const Image& b,float t) {
 Image out;
 for(int p=0;p<4096;p+=4) {
  float aa=a.rgba[p+3]*(1-t),ba=b.rgba[p+3]*t,alpha=aa+ba;
  for(int c=0;c<3;++c)out.rgba[p+c]=alpha>0?uint8_t(std::clamp((a.rgba[p+c]*aa+b.rgba[p+c]*ba)/alpha,0.f,255.f)):0;
  out.rgba[p+3]=uint8_t(std::clamp(alpha,0.f,255.f));
 }
 return out;
}
// Fractional sampling keeps the original art smooth during animated motion.
inline Image Transform(const Image& a,float scale,float dx=0,float dy=0) {
 Image out;
 for(int y=0;y<32;++y)for(int x=0;x<32;++x) {
  float sx=(x-15.5f-dx)/scale+15.5f,sy=(y-15.5f-dy)/scale+15.5f;
  int x0=int(std::floor(sx)),y0=int(std::floor(sy));float fx=sx-x0,fy=sy-y0;
  float color[4]{};
  for(int j=0;j<2;++j)for(int i=0;i<2;++i) {
   int xx=x0+i,yy=y0+j;if(xx<0||xx>=32||yy<0||yy>=32)continue;
   int p=(yy*32+xx)*4;float w=(i?fx:1-fx)*(j?fy:1-fy),alpha=a.rgba[p+3]/255.f;
   for(int c=0;c<3;++c)color[c]+=w*a.rgba[p+c]*alpha;
   color[3]+=w*a.rgba[p+3];
  }
  int p=(y*32+x)*4;out.rgba[p+3]=uint8_t(std::clamp(color[3],0.f,255.f));
  for(int c=0;c<3;++c)out.rgba[p+c]=color[3]>0?uint8_t(std::clamp(color[c]*255/color[3],0.f,255.f)):0;
 }
 return out;
}
constexpr float kAlternatePeriod=2.2f;
constexpr float kAnimationFPS=60.f;
inline float Fade(float phase) {
 phase=std::fmod(std::max(0.f,phase),kAlternatePeriod);
 float t=phase<.85f?0.f:phase<1.1f?(phase-.85f)/.25f:phase<1.95f?1.f:1.f-(phase-1.95f)/.25f;
 t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);
}
inline std::array<float,2> RotationOffset(float time) {
 const float phase=time*6.2831853f/3.f;
 return {3.f*std::sin(phase),3.f*std::sin(2.f*phase)};
}
inline std::vector<uint8_t> NativeTiles(const Image& a,int size) {
 std::vector<uint8_t> out(size*size*4);
 for(int y=0;y<size;++y)for(int x=0;x<size;++x) {
  const float sx=(x+.5f)*32/size-.5f,sy=(size-1-y+.5f)*32/size-.5f;
  int x0=int(std::floor(sx)),y0=int(std::floor(sy));float fx=sx-x0,fy=sy-y0;
  float col[4]{};
  for(int j=0;j<2;++j)for(int i=0;i<2;++i) {
   const int p=(std::clamp(y0+j,0,31)*32+std::clamp(x0+i,0,31))*4;
   const float weight=(i?fx:1-fx)*(j?fy:1-fy),alpha=a.rgba[p+3]/255.f;
   for(int c=0;c<3;++c)col[c]+=weight*a.rgba[p+c]*alpha;
   col[3]+=weight*a.rgba[p+3];
  }
  uint8_t pixel[4]{};pixel[3]=uint8_t(std::clamp(col[3],0.f,255.f));
  for(int c=0;c<3;++c)pixel[c]=col[3]>0?uint8_t(std::clamp(col[c]*255/col[3],0.f,255.f)):0;
  const int block=((y/4)*(size/4)+x/4)*64,offset=((y%4)*4+x%4)*2;
  out[block+offset]=pixel[3];out[block+offset+1]=pixel[0];
  out[block+32+offset]=pixel[1];out[block+32+offset+1]=pixel[2];
 }
 return out;
}
}
