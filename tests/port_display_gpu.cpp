#include <aurora/aurora.h>
#include "webgpu/gpu.hpp"
#include "port_display_controls.h"
#include <SDL3/SDL_video.h>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <cmath>
using namespace aurora::webgpu;
void Check(bool ok) { if(!ok) { std::fputs("Display GPU test failed\n",stderr); std::abort(); } }
int main(int argc,char** argv) {
  std::filesystem::create_directories("display-gpu-user");
  AuroraConfig config{};
  config.appName="Metroid display test";
  config.userPath="display-gpu-user"; config.cachePath="display-gpu-user";
  config.desiredBackend=BACKEND_AUTO; config.msaa=1; config.maxTextureAnisotropy=1;
  config.windowPosX=-1; config.windowPosY=-1; config.windowWidth=64;config.windowHeight=64;
  config.mem1Size=MEM1_DEFAULT_SIZE;config.mem2Size=ARAM_DEFAULT_SIZE;
  config.logLevel=LOG_WARNING;
  const auto info=aurora_initialize(argc,argv,&config);
  SDL_HideWindow(info.window);
  {
    const auto source=create_render_texture(4,4,false);
    const auto target=create_render_texture(4,4,false);
    const auto bindings=create_copy_bind_group(source);
    const wgpu::BufferDescriptor desc{.usage=wgpu::BufferUsage::CopyDst|wgpu::BufferUsage::MapRead,.size=1024};
    const auto readback=g_device.CreateBuffer(&desc);
    for(float percent : {0.f,50.f,50.5f,100.f}) {
      const float gain=PortDisplayControls::BrightnessGain(percent);
      aurora_set_display_brightness(gain);
      update_display_brightness();
      const auto cmd=g_device.CreateCommandEncoder();
      const wgpu::RenderPassColorAttachment clear{.view=source.view,.loadOp=wgpu::LoadOp::Clear,.storeOp=wgpu::StoreOp::Store,.clearValue={0.5,0.5,0.5,1.0}};
      const wgpu::RenderPassDescriptor clearDesc{.colorAttachmentCount=1,.colorAttachments=&clear};
      auto clearPass=cmd.BeginRenderPass(&clearDesc);clearPass.End();
      const wgpu::RenderPassColorAttachment dst{.view=target.view,.loadOp=wgpu::LoadOp::Clear,.storeOp=wgpu::StoreOp::Store};
      const wgpu::RenderPassDescriptor drawDesc{.colorAttachmentCount=1,.colorAttachments=&dst};
      auto pass=cmd.BeginRenderPass(&drawDesc);pass.SetPipeline(g_CopyPipeline);pass.SetBindGroup(0,bindings);pass.Draw(3);pass.End();
      const wgpu::TexelCopyTextureInfo from{.texture=target.texture};
      const wgpu::TexelCopyBufferInfo to{.layout={.bytesPerRow=256,.rowsPerImage=4},.buffer=readback};
      const wgpu::Extent3D extent{4,4,1};
      cmd.CopyTextureToBuffer(&from,&to,&extent);
      auto commands=cmd.Finish();g_queue.Submit(1,&commands);
      bool mapped=false;
      const auto future=readback.MapAsync(wgpu::MapMode::Read,0,1024,wgpu::CallbackMode::WaitAnyOnly,[&mapped](wgpu::MapAsyncStatus status,wgpu::StringView){mapped=status==wgpu::MapAsyncStatus::Success;});
      Check(g_instance.WaitAny(future,5000000000)==wgpu::WaitStatus::Success && mapped);
      const auto* pixel=static_cast<const unsigned char*>(readback.GetConstMappedRange());
      const int expected=static_cast<int>(std::round(128.f*gain));
      std::printf("Brightness %.1f: RGB %u %u %u, expected %d\n",percent,pixel[0],pixel[1],pixel[2],expected);
      for(int c=0;c<3;++c) Check(std::abs(static_cast<int>(pixel[c])-expected)<=1);
      Check(pixel[3]==255);
      readback.Unmap();
    }
  }
  aurora_shutdown();
  std::puts("Final presentation shader brightness passed");
}
