#include "Renderer.h"
#include "Utils.h"
#include <simd/simd.h>

#include <iostream>

MyMTKViewDelegate::MyMTKViewDelegate( MTL::Device* pDevice ) : MTK::ViewDelegate(), _pRenderer( new Renderer( pDevice ) ) {}

MyMTKViewDelegate::~MyMTKViewDelegate()
{
  delete _pRenderer;
}

Renderer::Renderer( MTL::Device* pDevice ) : _pDevice( pDevice->retain() )
{
  _pCommandQueue = _pDevice->newCommandQueue();
  buildShaders();
  buildBuffers();
}

Renderer::~Renderer()
{
  _pVertexPositionsBuffer->release();
  _pVertexColorsBuffer->release();
  _pPSO->release();
  _pCommandQueue->release();
  _pDevice->release();
}

void Renderer::buildShaders()
{
  using NS::StringEncoding::UTF8StringEncoding;

  const char* shaderSrc = R"(
    #include <metal_stdlib>
    using namespace metal;

    struct v2f
    {
      float4 position [[position]];
      half3 color;
    };

    vertex v2f vertexMain( uint vertexId [[vertex_id]], 
    device const float3* positions [[buffer(0)]],
    device const float3* colors [[buffer(1)]])
    {
      v2f o;
      o.position = float4( positions[ vertexId ], 1.0);
      o.color = half3( colors[ vertexId ] );
      return o;
    }

    fragment half4 fragmentMain( v2f in [[stage_in]] )
    {
      return half4( in.color, 1.0 );
    }
  )";

  NS::Error* pError = nullptr;
  MTL::Library* pLibrary = _pDevice->newLibrary( 
      NS::String::string( shaderSrc, UTF8StringEncoding ), nullptr, &pError );
  if( !pLibrary )
  {
    __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
    Assert( false );
  }

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

  _pPSO = _pDevice->newRenderPipelineState( pDesc, &pError );
  if( !_pPSO )
  {
    __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
    Assert( 0 );
  }

  pVertexFn->release();
  pFragFn->release();
  pDesc->release();
  pLibrary->release();

}

void Renderer::buildBuffers()
{
  const size_t NumVertices = 3;

  simd::float3 vertices[3] = 
  {
    { -0.75f, -0.75f, 1.0},
    { 0.75f, -0.75f, 1.0},
    { 0.0f, 0.75f, 1.0},
  };

  simd::float3 colors[3] = 
  {
    { 1.0f, 0.0, 0.0 },
    { 0.0f, 1.0, 0.0 },
    { 0.0f, 0.0, 1.0 },
  };
  
  const size_t SizeOfVertices = NumVertices * sizeof( simd::float3 );
  const size_t SizeOfColors = NumVertices * sizeof( simd::float3 );

  MTL::Buffer* VertexBuffer = _pDevice->newBuffer( SizeOfVertices, MTL::ResourceStorageModeShared);
  MTL::Buffer* VertexColorBuffer = _pDevice->newBuffer( SizeOfColors, MTL::ResourceStorageModeShared);

  _pVertexPositionsBuffer = VertexBuffer;
  _pVertexColorsBuffer = VertexColorBuffer;

  memcpy( _pVertexPositionsBuffer->contents(), vertices, SizeOfVertices );
  memcpy( _pVertexColorsBuffer->contents(), colors, SizeOfColors );
}

void Renderer::draw( MTK::View* pView )
{
  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* Command = _pCommandQueue->commandBuffer();
  MTL::RenderPassDescriptor* RenderPassDescriptor = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* Encoder = Command->renderCommandEncoder( RenderPassDescriptor );
  Encoder->setRenderPipelineState( _pPSO );
  Encoder->setVertexBuffer( _pVertexPositionsBuffer, 0, 0 );
  Encoder->setVertexBuffer( _pVertexColorsBuffer, 0, 1 );
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

