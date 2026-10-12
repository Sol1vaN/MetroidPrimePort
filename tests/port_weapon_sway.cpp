#include "port_weapon_sway.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
using namespace PortWeaponSway;
void Check(bool ok, const char* why) { if (!ok) { std::fprintf(stderr,"Sway: %s\n",why); std::abort(); } }
bool Near(double a,double b,double eps=1.e-9) { return std::abs(a-b)<eps; }
Pose Turn(int mode, int fps, double yawRate, double pitchRate, bool settle=false) {
  State s; s.Update(true,mode,1.0/fps,0,0);
  for(int i=1;i<=fps/2;i++) s.Update(true,mode,1.0/fps,yawRate*i/fps,pitchRate*i/fps);
  if(settle) for(int i=0;i<fps/2;i++) s.Update(true,mode,1.0/fps,yawRate*.5,pitchRate*.5);
  return s.GetPose();
}
int main() {
  for(int mode:{Stick,Inertia}) {
    const double sign=1;
    const auto r=Turn(mode,60,-1,0), l=Turn(mode,60,1,0);
    const auto u=Turn(mode,60,0,1), d=Turn(mode,60,0,-1);
    Check(r.right*sign>0 && r.yaw*sign<0,"right translation and barrel yaw");
    Check(u.up*sign>0 && u.pitch*sign>0,"up translation and barrel pitch");
    Check(Near(r.right,-l.right) && Near(u.up,-d.up),"symmetric left/right/up/down");
    Check(Near(r.up,0) && Near(u.right,0),"no cross-axis drift");
    for(int fps:{30,60,120,144,240}) {
      const auto p=Turn(mode,fps,-1,.4), reference=Turn(mode,60,-1,.4);
      Check(Near(p.right,reference.right,0.001) && Near(p.pitch,reference.pitch,0.001),"FPS independent sustained look");
      const auto rest=Turn(mode,fps,-1,0,true);
      Check(std::abs(rest.right)<.0003 && std::abs(rest.pitch)<.0002,"settles after release");
    }
    State s; s.Update(true,mode,1./240,0,0);
    double yaw=0,pitch=0;
    for(int i=0;i<2400;++i) {
      yaw+=(i%100<50?-1:1)*.8; pitch+=(i%60<30?-1:1)*.5;
      auto p=s.Update(true,mode,1./240,yaw,pitch);
      Check(std::isfinite(p.yaw) && std::hypot(s.horizontal.x,s.vertical.x)<.20,"flicks remain finite and bounded");
    }
    s.Update(false,mode,1./60,0,0);
    Check(Near(s.GetPose().right,0) && !s.initialized,"inactive camera resets");
    auto first=s.Update(true,mode,1./60,2,1);
    Check(Near(first.right,0) && Near(first.up,0),"resume rebases without kick");
    s.Update(true,mode,.5,0,0);
    Check(!s.initialized,"long stalls rebase");
    s.Update(true,mode,1./60,std::numeric_limits<double>::quiet_NaN(),0);
    Check(!s.initialized,"invalid input safe");
  }
  for(int mode:{Stick,Inertia}) {
    State pointing; pointing.Update(true,mode,.01,0,.8);
    for(int i=0;i<300;++i)pointing.Update(true,mode,.01,0,.8);
    Check(Near(pointing.horizontal.x,0) && Near(pointing.vertical.x,0),"static look has no turn-rate sway");
    if(mode==Inertia) Check(pointing.GetPose().pitch>.04 && pointing.GetPose().pitch<.06,"gentle absolute look pointing survives idle");
    else Check(Near(pointing.GetPose().pitch,0),"Stick does not inherit inertia look pointing");
    for(int i=0;i<400;++i)pointing.Update(true,mode,.01,0,0);
    Check(std::abs(pointing.GetPose().pitch)<1.e-6,"pointing returns to neutral at horizon");
  }
  const auto gesture=Turn(Stick,60,-2,.5);
  Check(gesture.right>0 && gesture.right<.01 && gesture.elbowWeight>.99 && gesture.forward<0 && gesture.roll<0,"stick translation, shoulder pivot, depth arc and roll");
  // Test the exact rigid overlay used by Render: a rear elbow and muzzle
  // one arm-length ahead. The muzzle must travel much farther than the rear.
  for (const auto pose : {Turn(Stick,60,-2,0), Turn(Stick,60,2,0),
                          Turn(Stick,60,0,2), Turn(Stick,60,0,-2)}) {
    const std::array<double,3> elbow={.25,-.1,-.35};
    const auto m=VisualTransform(pose,elbow,{0,0,0});
    const auto shifted=VisualTransform(pose,elbow,{.1,-.2,.15});
    double rearDistance=0, muzzleDistance=0;
    for(int i=0;i<3;++i) {
      const double rear=m[4*i]*elbow[0]+m[4*i+1]*elbow[1]+m[4*i+2]*elbow[2]+m[4*i+3];
      const double rearDelta=rear-elbow[i];
      const double muzzleDelta=rearDelta+.65*(m[4*i+1]-(i==1?1.:0.));
      rearDistance+=rearDelta*rearDelta;
      muzzleDistance+=muzzleDelta*muzzleDelta;
      for(int j=0;j<3;++j) Check(Near(m[4*i+j],shifted[4*i+j]),"placement preserves barrel orientation");
    }
    Check(std::sqrt(rearDistance)<.012,"rear arm anchor stays nearly stationary");
    Check(muzzleDistance>25*rearDistance,"muzzle arc dominates rear movement");
    Check(std::acos(m[5])>.16,"visible pointing retained at new 80 percent baseline");
    Check(m[1]*pose.right>=0 && m[9]*pose.up>=0,"muzzle points with mouse gesture");
  }
  // Measurable easing: reduced initial response, retained momentum on release.
  for(int fps:{30,60,120,240}) {
    State smooth; smooth.Update(true,Stick,1.0/fps,0,0);
    Spring prior;
    const double target=.18*std::tanh(.065/.18);
    for(int i=1;i<=fps/10;++i) {
      smooth.Update(true,Stick,1.0/fps,-double(i)/fps,0);
      prior.Step(target,22,1,1.0/fps);
    }
    Check(smooth.horizontal.x < prior.x*.7,"Stick absorbs initial mouse acceleration");
    const double before=smooth.horizontal.x;
    smooth.Update(true,Stick,1.0/fps,-double(fps/10)/fps,0);
    Check(smooth.horizontal.x>before,"Stick retains momentum on release");
  }
  for(int mode:{Stick,Inertia}) {
    State power;power.Update(true,mode,.01,0,0);
    for(int i=1;i<=50;++i)power.Update(true,mode,.01,-i*.02,i*.005);
    const auto normal=power.GetPose(), half=power.GetPose(.5), twice=power.GetPose(2);
    Check(Near(half.yaw,normal.yaw*.5) && Near(half.pitch,normal.pitch*.5) &&
          Near(twice.up,normal.up*2) && Near(twice.right,normal.right*2) &&
          Near(twice.roll,normal.roll*2) && Near(twice.forward,normal.forward*2),"strength scales all gesture components");
    Check(Near(half.elbowWeight,normal.elbowWeight) && Near(twice.elbowWeight,normal.elbowWeight),"strength preserves pivot");
    const auto zero=VisualTransform(power.GetPose(0),{.25,-.1,-.35},{0,0,0});
    for(int i=0;i<12;++i)Check(Near(zero[i],i==0||i==5||i==10?1.:0.),"zero strength is an identity overlay");
    Check(Near(power.GetPose(-1).yaw,0) && Near(power.GetPose(10).yaw,twice.yaw) &&
          Near(power.GetPose(std::numeric_limits<double>::quiet_NaN()).yaw,normal.yaw),"invalid strength is bounded");
  }
  // Sustained look pointing rotates about the rear anchor after mouse motion stops.
  for(double pitch:{-.8,.8}) {
    State pointing;pointing.Update(true,Inertia,.01,0,pitch);
    for(int i=0;i<300;++i)pointing.Update(true,Inertia,.01,0,pitch);
    const auto pose=pointing.GetPose();
    Check(pose.elbowWeight==1 && std::abs(pose.pitch)>.04 && std::abs(pose.pitch)<.06,"Inertia holds visible pitch at rear anchor");
    Check(std::abs(pose.up)<.003,"pointing is rotation, not vertical sliding");
    const auto m=VisualTransform(pose,{.25,-.1,-.35},{0,0,0});
    Check(m[9]*pitch>0 && std::abs(m[9])>.04,"muzzle points with sustained look elevation");
  }
  const auto inertiaTurn=Turn(Inertia,60,-2,0),stickTurn=Turn(Stick,60,-2,0);
  Check(std::abs(inertiaTurn.yaw)<std::abs(stickTurn.yaw)*.57,"Inertia and Stick have distinct pointing amplitudes");
  State calibrated;
  calibrated.initialized=true;calibrated.stickBlend.x=1;calibrated.horizontal.x=.1;
  Check(Near(calibrated.GetPose().yaw,-.1*2.4*.8),"Stick 100 percent is former 80 percent");
  State disabled;disabled.Update(true,Inertia,.01,0,0);disabled.Update(true,Inertia,.01,-.1,.1);
  const auto none=disabled.Update(true,None,.01,-.2,.2);
  Check(!disabled.initialized && Near(none.right,0) && Near(none.up,0) &&
        Near(none.yaw,0) && Near(none.pitch,0) && Near(none.roll,0),"None clears all residual movement");
  const auto resumed=disabled.Update(true,Inertia,.01,2,1);
  Check(Near(resumed.right,0) && Near(resumed.up,0),"enabling after None rebases input");
  State seam, regular;
  seam.Update(true,Stick,1./60,kPi-.01,0); regular.Update(true,Stick,1./60,0,0);
  seam.Update(true,Stick,1./60,-kPi+.01,0); regular.Update(true,Stick,1./60,.02,0);
  Check(Near(seam.GetPose().right,regular.GetPose().right),"yaw wrap has no spike");
  const auto before=seam.GetPose();
  const auto after=seam.Update(true,Inertia,0,0,0);
  Check(Near(before.right,after.right),"mode switch preserves pose continuity");
  State idle;idle.Update(true,Stick,.01,0,0);
  for(int i=0;i<1000;i++)idle.Update(true,Stick,.01,0,0);
  Check(Near(idle.GetPose().right,0) && Near(idle.GetPose().pitch,0),"idle animations get an identity overlay");
  std::puts("Weapon sway: directions, frame rates, flicks, settling, wrap and suspension passed");
}
