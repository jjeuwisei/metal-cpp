#include "Renderer.h" 
#include "Utils.h"
#include <simd/simd.h>
#include <cstdint>
#include <iostream>

struct Vertex
{
    simd::float4 position;
    simd::float4 color;
};

static const Vertex Vertices[] =
{
    // Position                     // Color
    {{ 0.0f,  0.5f, 0.0f, 1.0f},   {1.0f, 0.0f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.0f, 1.0f},   {0.0f, 1.0f, 0.0f, 1.0f}},
    {{ 0.5f, -0.5f, 0.0f, 1.0f},   {0.0f, 0.0f, 1.0f, 1.0f}},
};

static const uint16_t Indices[] = 
{
  0, 1, 2
};

MyMTKViewDelegate::MyMTKViewDelegate( MTL::Device* pDevice ) : MTK::ViewDelegate(), _pRenderer( new Renderer( pDevice ) ) {}

MyMTKViewDelegate::~MyMTKViewDelegate()
{
  delete _pRenderer;
}

Renderer::Renderer( MTL::Device* pDevice ) : _pDevice( pDevice->retain() )
{
  CommandQueue = _pDevice->newCommandQueue();
  BuildRenderPipeline();
  BuildBuffers();
}

Renderer::~Renderer()
{
  IndexBuffer->release();
  VertexBuffer->release();
  _pShaderLibrary->release();
  _pPSO->release();
  CommandQueue->release();
  _pDevice->release();
}

MTL::Library* Renderer::LoadLibrary()
{
  NS::String* path = NS::String::string("./build/Shader.metallib", NS::StringEncoding::UTF8StringEncoding);
  NS::URL* url = NS::URL::fileURLWithPath(path);

  NS::Error* error = nullptr;
  MTL::Library* library = _pDevice->newLibrary(url, &error);
  if(!library)
  {
    __builtin_printf("%s", error->localizedDescription()->utf8String());
  }
  return library;
}

void Renderer::BuildRenderPipeline()
{
  using NS::StringEncoding::UTF8StringEncoding;

  MTL::Library* pLibrary = LoadLibrary();

  MTL::Function* pVertexFn = pLibrary->newFunction( NS::String::string( "vertexMain", UTF8StringEncoding) );
  if( !pVertexFn )
  {
    std::cout << "Error Creating Vertex Function" << std::endl;
    Assert( false );
  }
  MTL::Function* pFragFn = pLibrary->newFunction( NS::String::string( "fragmentMain", UTF8StringEncoding ) );
  if( !pFragFn )
  {
    std::cout << "Error Creating fragment Function" << std::endl;
    Assert( false );
  }

  MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
  pDesc->setVertexFunction( pVertexFn );
  pDesc->setFragmentFunction( pFragFn );
  pDesc->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB );
  if(!pDesc)
  {
    std::fprintf(stderr, "failed to create render pipeline descriptor\n");
    Assert(false);
  }

  MTL::VertexDescriptor* VertexDesc = MTL::VertexDescriptor::alloc()->init();
  VertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat4);
  VertexDesc->attributes()->object(0)->setOffset(offsetof(Vertex, position));
  VertexDesc->attributes()->object(0)->setBufferIndex(0);
  VertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatFloat4);
  VertexDesc->attributes()->object(1)->setOffset(offsetof(Vertex, color));
  VertexDesc->attributes()->object(1)->setBufferIndex(0);
  VertexDesc->layouts()->object(0)->setStride(sizeof(Vertex));
  VertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunction::VertexStepFunctionPerVertex);
  if(!VertexDesc)
  {
    std::fprintf(stderr, "failed to create vertex descriptor\n");
    Assert(false);
  }

  pDesc->setVertexDescriptor(VertexDesc);

  NS::Error* pError = nullptr;
  _pPSO = _pDevice->newRenderPipelineState( pDesc, &pError );
  if( !_pPSO )
  {
    __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
    Assert( 0 );
  }

  pVertexFn->release();
  pFragFn->release();
  VertexDesc->release();
  pDesc->release();
  _pShaderLibrary = pLibrary;
}

void Renderer::BuildBuffers()
{  
  VertexBuffer = _pDevice->newBuffer(Vertices, sizeof(Vertices), MTL::ResourceStorageModeShared);
  IndexBuffer = _pDevice->newBuffer(Indices, sizeof(Indices), MTL::ResourceStorageModeShared); 
  if(!VertexBuffer || !IndexBuffer)
  {
    std::fprintf(stderr, "error creating Buffer\n");
    Assert(false);
  }
}

void Renderer::draw( MTK::View* pView )
{
  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* Command = CommandQueue->commandBuffer();
  MTL::RenderPassDescriptor* RenderPassDescriptor = pView->currentRenderPassDescriptor();

  MTL::RenderCommandEncoder* Encoder = Command->renderCommandEncoder( RenderPassDescriptor );
  Encoder->setRenderPipelineState( _pPSO );
  Encoder->setVertexBuffer(VertexBuffer, 0, 0);
  Encoder->drawIndexedPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle, 3, MTL::IndexType::IndexTypeUInt16, IndexBuffer, 0);
  Encoder->endEncoding();

  Command->presentDrawable( pView->currentDrawable() );
  Command->commit();

  pool->release();
}

void MyMTKViewDelegate::drawInMTKView( MTK::View* pView )
{
  _pRenderer->draw( pView );
}

