// Drive the actual MusyX macro, voice allocation, detent bridge and PC mixer.
// A dummy SDL device keeps this regression silent and independent of game data.
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <musyx/hardware.h>
#include <musyx/dspvoice.h>
#include <musyx/synthdata.h>
#include <musyx/synth.h>
#include <musyx/port_voices.h>
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
static void* alloc(size_t n) { return malloc(n); }
static void release(void* p) { free(p); }
static SDIR_DATA directory[2];
static unsigned char pcm[2048];
// Same volume-from-start-velocity instruction used by the map Zoom macro.
static MSTEP macro[]={
  {{0xff002d0d,0x000001ff}}, // scale volume from original velocity by 45/127
  {{0x00032110,0x00000000}}, // start synthetic sample 0x321
  {{0x01000107,0xffff0000}}, // wait for key-off
  {{0x00000011,0x00000000}}, // stop sample
  {{0x00000000,0x00000000}}
};
static FX_TAB fx={0x322,0x323,4,250,127,64,60,0};

static void runCase(int startVolume,int expectAudible) {
  MusyxPortBeginVoiceUpdate();
  SND_VOICEID voice=sndFXStartEx(0x322,startVolume,64,0);
  CHECK(voice!=SND_ID_ERROR);
  MusyxPortSetVoiceStepTravel(voice,0,.005f);
  sndFXVolume(voice,127); // reproduces the failing "raise volume after start" path
  MusyxPortEndVoiceUpdate();
  int audible=0,running=0,maxRms=0,maxVol=0;
  for(int i=0;i<100;++i) {
    MusyxPortSetVoiceStepTravel(voice,.2f,.005f); // 40 clicks/s
    SDL_Delay(5);
    PortMusyxVoice voices[8];int count=MusyxPortCopyVoices(voices,8);
    for(int j=0;j<count;++j)if(voices[j].smpId==0x321) {
      ++running;
      if(voices[j].rms>maxRms)maxRms=voices[j].rms;
      if(voices[j].volL>maxVol)maxVol=voices[j].volL;
      if(voices[j].rms>20 && (voices[j].volL>0 || voices[j].volR>0))++audible;
    }
  }
  hwDisableIrq();
  for(int i=0;i<8;++i)if(dspVoice[i].state)printf("voice %d org=%u vol=%u lastL=%u currL=%u group=%d\n",i,synthVoice[i].orgVolume,synthVoice[i].volume,dspVoice[i].lastVolL,dspVoice[i].volL,synthVoice[i].vGroup);
  hwEnableIrq();
  printf("running=%d rms=%d vol=%d\n",running,maxRms,maxVol);
  CHECK(running>0);
  printf("start volume %d: %d audible observations\n",startVolume,audible);
  CHECK(expectAudible ? audible>0 : audible==0);
  if(expectAudible) {
    // Finish the last command and its last native fragment, then measure real mixer silence.
    SDL_Delay(180);
    for(int i=0;i<20;++i) {
      PortMusyxVoice voices[8];int count=MusyxPortCopyVoices(voices,8);
      for(int j=0;j<count;++j)if(voices[j].smpId==0x321)CHECK(voices[j].rms==0);
      SDL_Delay(5);
    }
  }
  sndFXKeyOff(voice);SDL_Delay(40);
}
int main(void) {
  setvbuf(stdout,NULL,_IONBF,0);
  SDL_setenv_unsafe("SDL_AUDIODRIVER","dummy",1);
  SND_HOOKS hooks={alloc,release};sndSetHooks(&hooks);
  CHECK(sndInit(8,0,8,1,0,1024*1024)==0);
  sndMasterVolume(127,0,1,1);for(int g=0;g<32;++g)sndVolume(127,0,g);
  for(int i=0;i<1024;++i) {
    short v=(short)(12000*sin(6.283185307179586*i/32));
    pcm[2*i]=(unsigned short)v>>8;pcm[2*i+1]=(unsigned short)v&255;
  }
  directory[0].id=0x321;
  directory[0].header.info=(60u<<24)|32000;
  directory[0].header.length=(2u<<24)|1024; // native BE PCM16
  directory[0].header.loopLength=1024;directory[1].id=0xffff;
  hwDisableIrq();
  CHECK(dataInsertSDir(directory,pcm));
  CHECK(dataAddSampleReference(0x321
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
    ,dataARAMDefaultGetInfo()
#endif
  ));
  CHECK(dataInsertMacro(0x323,macro));CHECK(dataInsertFX(0x324,&fx,1));
  hwEnableIrq();
  runCase(0,0);
  runCase(127,1);
  sndQuit();SDL_Quit();
  puts("Map SFX native startup and detent mixer regression passed");
  return 0;
}
