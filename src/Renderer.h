#ifndef RENDERER_H
#define RENDERER_H

#include <Appkit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

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
#endif
