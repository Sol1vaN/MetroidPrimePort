#include <musyx/pc_audio_math.h>
#include <musyx/pc_step_tempo.h>
#include <stdio.h>
#include <stdlib.h>

#define check(condition) do { if (!(condition)) { fprintf(stderr,"Failed line %d: %s\n",__LINE__,#condition); abort(); } } while(0)

static void testStepDetents(void) {
  const int frames[]={30,60,144,240};
  const double rates[]={.5,2,8,20,MUSYX_MAP_STEP_NATIVE_HZ,MUSYX_MAP_STEP_MAX_HZ};
  for(int f=0;f<4;++f)for(int r=0;r<6;++r) {
    MusyxStepDetents state={0};int sample=0,lastClick=-1,interval=0;
    for(int frame=0;frame<frames[f]*4;++frame) {
      musyxStepPush(&state,rates[r]/frames[f],1.0/frames[f]);
      int end=(int)llround((frame+1)*32000.0/frames[f]);
      for(;sample<end;++sample) {
        if(musyxStepAdvance(&state,1.0/32000)) {
          if(lastClick>=0){interval=sample-lastClick;check(abs(interval-(int)(32000/rates[r]))<=2);}
          lastClick=sample;
        }
      }
    }
    printf("%d FPS: %.3f teeth/s -> %u clicks / 4s, interval %.3f ms\n",frames[f],rates[r],state.clicks,interval/32.0);
    check(abs((int)state.clicks-(int)(4*rates[r]))<=1);
    for(int i=0;i<800;++i)musyxStepAdvance(&state,1.0/32000);
    unsigned count=state.clicks;
    for(int i=0;i<32000;++i)musyxStepAdvance(&state,1.0/32000);
    check(state.clicks==count);
  }
  MusyxStepDetents state={0};
  musyxStepPush(&state,1000,.05);
  for(int i=0;i<1600;++i)musyxStepAdvance(&state,1.0/32000);
  check(state.clicks==3); // The measured native cadence times the 130% ceiling.

  // At 100% every native PCM sample survives exactly, after a fixed 593-sample
  // buffer delay. A non-periodic input catches lost/repeated grains and gating.
  memset(&state,0,sizeof(state));
  int32_t history[64000];
  for(int i=0;i<64000;++i)history[i]=(int32_t)((i*7919u)%60001u)-30000;
  for(int block=0;block<400;++block) {
    int32_t pcm[160];memcpy(pcm,history+block*160,sizeof(pcm));
    musyxStepPush(&state,MUSYX_MAP_STEP_NATIVE_HZ*.005,.005);
    musyxStepApply(&state,32000,pcm,160);
    for(int i=0;i<160;++i) {
      int n=block*160+i;
      check(pcm[i]==(n>=593?history[n-593]:0));
    }
  }
  // Copying each grain must keep a 1 kHz tone at 1 kHz, even at 25% and 130%.
  for(int r=0;r<3;++r) {
    const double ratios[]={.25,1,MUSYX_MAP_STEP_MAX_RATIO};
    memset(&state,0,sizeof(state));int lastCross=-1,crossings=0,previous=0;
    for(int n=0;n<32000;++n) {
      if(n%160==0)musyxStepPush(&state,MUSYX_MAP_STEP_NATIVE_HZ*ratios[r]*.005,.005);
      int32_t pcm=(int32_t)(12000*sin(6.283185307179586*1000*n/32000));
      musyxStepApply(&state,32000,&pcm,1);
      int interior=0,active=0;
      for(int j=0;j<2;++j) {
        MusyxStepGrain* g=&state.grains[j];
        if(g->position<g->length) {
          ++active;
          if(g->position>100 && g->position<g->length-100)interior=1;
        }
      }
      if(active==1 && interior) {
        if(previous<=0 && pcm>0) {
          if(lastCross>=0){check(abs(n-lastCross-32)<=1);++crossings;}
          lastCross=n;
        }
      } else lastCross=-1;
      previous=pcm;
    }
    check(crossings>20);
    unsigned clicks=state.clicks;
    for(int block=0;block<30;++block) {
      int32_t pcm[160];for(int i=0;i<160;++i)pcm[i]=1000;
      musyxStepApply(&state,32000,pcm,160);
      if(block>4)for(int i=0;i<160;++i)check(pcm[i]==0);
    }
    check(state.clicks==clicks);
  }
}

int main(void) {
  setvbuf(stdout,NULL,_IONBF,0);
  check(musyxPcScaleQ15(100000, 32767) == 99996);
  check(musyxPcScaleQ15(-100000, 32767) == -99997);
  check(musyxPcScaleQ15(INT32_MAX, 65535) == INT32_MAX);
  check(musyxPcClamp32((int64_t)INT32_MAX + 1) == INT32_MAX);
  check(musyxPcClamp32((int64_t)INT32_MIN - 1) == INT32_MIN);
  check(musyxPcPcm8(0) == 0);
  check(musyxPcPcm8(127) == 32512);
  check(musyxPcPcm8(128) == -32768);
  check(musyxPcPcm8(255) == -256);

  int16_t coefficients[8][2] = {{2048, 0}}; // previous sample plus signed nibble
  const uint8_t block[8] = {0, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
  int16_t h1 = 0, h2 = 0;
  const unsigned end = musyxPcLoopEnd(14, 5, 5);
  check(end == 10); // do not play the tail after the loop end
  unsigned position = 0;
  for (unsigned output = 0; output < 30; ++output) {
    if (position == end) {
      position = 5;
      h1 = 5;
      h2 = 4;
    }
    const int sample = musyxPcAdpcmSample(block, position++, 0, coefficients, &h1, &h2);
    check(sample == (output < 10 ? (int)output + 1 : 6 + (int)(output - 10) % 5));
  }
  // Ring-buffer ADPCM is continuous: retaining history produces 15, not 1,
  // when the next buffer block follows the first fourteen samples.
  h1 = h2 = 0;
  for (unsigned i = 0; i < 14; ++i) musyxPcAdpcmSample(block, i, 0, coefficients, &h1, &h2);
  check(musyxPcStreamedAdpcm(4) && musyxPcStreamedAdpcm(5) && !musyxPcStreamedAdpcm(0));
  check(musyxPcAdpcmSample(block, 0, 0, coefficients, &h1, &h2) == 15);
  check(musyxPcLoopEnd(100, 14, 28) == 42);
  check(musyxPcLoopEnd(100, 0, 0) == 100);
  check(musyxPcLoopEnd(UINT32_MAX, UINT32_MAX - 2, 10) == UINT32_MAX);
  testStepDetents();
  puts("ADPCM loop, signed PCM8 and wide-mix regressions passed");
  return 0;
}
