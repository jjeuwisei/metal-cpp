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
 void buildShaders();
 void buildBuffers();

 private:
 MTL::Device* _pDevice;
 MTL::CommandQueue* _pCommandQueue;
 MTL::RenderPipelineState* _pPSO;
 MTL::Buffer* _pVertexPositionsBuffer;
 MTL::Buffer* _pVertexColorsBuffer;
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
