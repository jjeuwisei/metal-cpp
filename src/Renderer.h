#ifndef RENDERER_H
#define RENDERER_H

#include <Appkit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

class Renderer
{
 public:
 Renderer( MTL::Device* pDevice);
 ~Renderer();
 void draw( MTK::View* pView );
 void BuildRenderPipeline();
 void BuildBuffers();
 bool Initiliaze();
 MTL::Library* LoadLibrary();

 private:
 MTL::Device* _pDevice;
 MTL::CommandQueue* CommandQueue;
 MTL::RenderPipelineState* _pPSO;
 MTL::Library* _pShaderLibrary;
 MTL::Buffer* VertexBuffer;
 MTL::Buffer* IndexBuffer = nullptr;
 MTL::RenderPipelineDescriptor* VertexDescriptor = nullptr;
 MTL::DepthStencilState* DepthState = nullptr;
};

class MyMTKViewDelegate : public MTK::ViewDelegate
{
  public:
  MyMTKViewDelegate( MTL::Device* pDevice );
  virtual ~MyMTKViewDelegate() override;
  virtual void drawInMTKView( MTK::View* pView ) override;

  private:
  Renderer* _pRenderer;
};

static simd_float4x4 RotationY(float angle);
static simd_float4x4 Translation(float x, float y, float z);
static simd_float4x4 Perspective(float fovY, float aspect, float nearZ, float farZ);

#endif
