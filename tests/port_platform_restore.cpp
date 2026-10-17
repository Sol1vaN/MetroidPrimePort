#include "port_platform_restore.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <vector>
void Check(bool ok,const char* msg) { if(!ok) {std::fprintf(stderr,"%s\n",msg);std::abort();} }
int main() {
  // Parent platform -> child platform -> effect, plus a direct effect. Simulate
  // the same recursive translation used by live DragSlaves and the restore.
  for(float sign:{-1.f,1.f}) for(float offset:{.1f,5.f,50.f}) {
    std::array<int,2> permutation{0,1};
    do {
      float position[4]={0,3,4,2};
      const float saved[2]={sign*offset,sign*offset+3};
      auto move=[&](auto&& self,int actor,float delta)->void {
        position[actor]+=delta;
        if(actor==0) {self(self,1,delta);self(self,3,delta);}
        if(actor==1) self(self,2,delta);
      };
      auto apply=[&](std::size_t i) {
        const int actor=permutation[i];
        const float delta=saved[actor]-position[actor];
        move(move,actor,delta);
      };
      auto parent=[&](std::size_t p,std::size_t c) {return permutation[p]==0 && permutation[c]==1;};
      Check(PortPlatformRestore::ParentsFirst(2,parent,apply),"valid nested restore");
      Check(position[0]==saved[0] && position[1]==saved[1],"saved absolute platform poses preserved regardless of snapshot order");
      Check(position[2]==saved[1]+1 && position[3]==saved[0]+2,"both slave effects retain correct relative offsets");
      const auto previous=std::array<float,4>{position[0],position[1],position[2],position[3]};
      Check(PortPlatformRestore::ParentsFirst(2,parent,apply),"repeated restore accepted");
      for(int a=0;a<4;++a) Check(position[a]==previous[a],"no double delta on repeated quickload");
    } while(std::next_permutation(permutation.begin(),permutation.end()));
  }
  int changes=0;
  Check(!PortPlatformRestore::ParentsFirst(3,[](std::size_t p,std::size_t c){return (p+1)%3==c;},[&](std::size_t){++changes;}),"cycle is rejected");
  Check(changes==0,"cycle rejection is atomic");
  Check(PortPlatformRestore::ParentsFirst(0,[](std::size_t,std::size_t){return false;},[&](std::size_t){++changes;}),"old states without platform poses remain valid");
  std::puts("Platform restore: nested effects, ordering, up/down motion, repeated loads and cycles passed");
}
