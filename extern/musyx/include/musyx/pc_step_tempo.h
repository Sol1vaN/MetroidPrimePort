#ifndef MUSYX_PC_STEP_TEMPO_H
#define MUSYX_PC_STEP_TEMPO_H
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "pc_map_step_timing.h"

#define MUSYX_STEP_QUEUE 16
#define MUSYX_STEP_GRAIN_MAX 2048
// Grain duration is measured from the native SFX, not an invented pulse window.
// Samples inside a grain are copied 1:1. Only the spacing between grains changes.
typedef struct MusyxStepSegment { double seconds, teethPerSecond; } MusyxStepSegment;
typedef struct MusyxStepGrain {
  int32_t pcm[MUSYX_STEP_GRAIN_MAX];
  int position, length;
  double edge;
} MusyxStepGrain;
typedef struct MusyxStepDetents {
  MusyxStepSegment queue[MUSYX_STEP_QUEUE];
  int head, count;
  double pendingSeconds, phase, rate;
  unsigned clicks;
  int32_t capture[MUSYX_STEP_GRAIN_MAX], ready[MUSYX_STEP_GRAIN_MAX];
  int captureCount, readyCount;
  MusyxStepGrain grains[2];
  int nextGrain;
} MusyxStepDetents;

static inline void musyxStepPush(MusyxStepDetents* s,double teeth,double dt) {
  if(!isfinite(teeth) || !isfinite(dt) || teeth<0 || dt<=0 || dt>.1)return;
  // Drop stale commands after a hitch; never replay a backlog of clicks.
  if(s->count==MUSYX_STEP_QUEUE || s->pendingSeconds+dt>.12) {
    s->head=s->count=0;s->pendingSeconds=0;
  }
  int tail=(s->head+s->count)%MUSYX_STEP_QUEUE;
  s->queue[tail].seconds=dt;
  s->queue[tail].teethPerSecond=fmin(MUSYX_MAP_STEP_MAX_HZ,teeth/dt);
  s->pendingSeconds+=dt;++s->count;
}
static inline int musyxStepAdvance(MusyxStepDetents* s,double dt) {
  double remaining=dt;int click=0;s->rate=0;
  while(remaining>1e-12 && s->count) {
    MusyxStepSegment* seg=&s->queue[s->head];
    double used=fmin(remaining,seg->seconds);s->rate=seg->teethPerSecond;
    s->phase+=used*s->rate;
    if(s->phase>=1.0-1e-9) {
      s->phase=fmax(0.0,s->phase-1.0);++s->clicks;click=1;
    }
    remaining-=used;seg->seconds-=used;s->pendingSeconds=fmax(0.0,s->pendingSeconds-used);
    if(seg->seconds<1e-12){s->head=(s->head+1)%MUSYX_STEP_QUEUE;--s->count;}
  }
  return click;
}
static inline void musyxStepApply(MusyxStepDetents* s,int sampleRate,int32_t* pcm,int count) {
  if(sampleRate<=0)return;
  const int length=(int)lround(sampleRate/MUSYX_MAP_STEP_NATIVE_HZ);
  if(length<2 || length>MUSYX_STEP_GRAIN_MAX) {memset(pcm,0,count*sizeof(*pcm));return;}
  for(int i=0;i<count;++i) {
    s->capture[s->captureCount++]=pcm[i];
    if(s->captureCount==length) {
      memcpy(s->ready,s->capture,length*sizeof(*pcm));s->readyCount=length;s->captureCount=0;
    }
    if(musyxStepAdvance(s,1.0/sampleRate) && s->readyCount) {
      MusyxStepGrain* g=&s->grains[s->nextGrain];s->nextGrain^=1;
      memcpy(g->pcm,s->ready,s->readyCount*sizeof(*pcm));g->length=s->readyCount;g->position=0;
      const double gap=sampleRate/s->rate-g->length;
      // At native cadence there is no windowing: consecutive fragments join
      // sample-for-sample. Slower steps have quiet gaps and short de-click ramps;
      // flicks overlap/crossfade, at most 130%, with no resampling or pitch shift.
      g->edge=fabs(gap)<1.0?0:(gap>0?fmin(gap,sampleRate*.001):-gap);
    }
    double sum=0,weight=0;
    for(int j=0;j<2;++j) {
      MusyxStepGrain* g=&s->grains[j];
      if(g->position>=g->length)continue;
      double gain=g->edge>0?fmin(1.0,fmin((g->position+1)/g->edge,(g->length-g->position)/g->edge)):1.0;
      sum+=g->pcm[g->position++]*gain;weight+=gain;
    }
    // Smooth transitions can briefly overlap unlike-sized windows. Do not
    // amplify them, and retain the fades into silence when weight is below 1.
    pcm[i]=(int32_t)(sum/fmax(1.0,weight));
  }
}
#endif
