#include "Renderer.h" 
#include "Utils.h"
#include <simd/simd.h>

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

  NS::Error* pError = nullptr;
  _pPSO = _pDevice->newRenderPipelineState( pDesc, &pError );
  if( !_pPSO )
  {
    __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
    Assert( 0 );
  }

  pVertexFn->release();
  pFragFn->release();
  pDesc->release();
  _pShaderLibrary = pLibrary;
}

void Renderer::BuildBuffers()
{  
  VertexBuffer = _pDevice->newBuffer(Vertices, sizeof(Vertices), MTL::ResourceStorageModeShared);
  if(!VertexBuffer)
  {
    std::fprintf(stderr, "error creating vertexbuffer \n");
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
  Encoder->drawPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3) );
  Encoder->endEncoding();

  Command->presentDrawable( pView->currentDrawable() );
  Command->commit();

  pool->release();
}

void MyMTKViewDelegate::drawInMTKView( MTK::View* pView )
{
  _pRenderer->draw( pView );
}

