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
{    // Front face
  {{-0.5f, -0.5f, -0.5f, 1.0f}, {1, 0, 0, 1}}, //front bot left
  {{0.5f, -0.5f, -0.5f, 1.0f}, {0, 1, 0, 1}}, //front bot righ
  {{0.5f,  0.5f, -0.5f, 1.0f}, {0, 0, 1, 1}}, //front top righ
  {{-0.5f,  0.5f, -0.5f, 1.0f}, {1, 1, 0, 1}}, //front top left
  // Back face               
  {{-0.5f, -0.5f,  0.5f, 1.0f}, {1, 0, 1, 1}}, //back bot left
  {{0.5f, -0.5f,  0.5f, 1.0f}, {0, 1, 1, 1}}, //back bot right
  {{0.5f,  0.5f,  0.5f, 1.0f}, {1, 1, 1, 1}}, //back top right
  {{-0.5f,  0.5f,  0.5f, 1.0f}, {0, 0, 0, 1}}, //back top left  
};

static const uint16_t Indices[] =
{
    // z = -0.5 face
    0, 2, 1,
    0, 3, 2,

    // z = +0.5 face
    4, 5, 6,
    4, 6, 7,

    // x = -0.5 face
    4, 7, 3,
    4, 3, 0,

    // x = +0.5 face
    1, 2, 6,
    1, 6, 5,

    // y = -0.5 face
    4, 0, 1,
    4, 1, 5,

    // y = +0.5 face
    3, 7, 6,
    3, 6, 2
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
  MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
  depthDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
  depthDesc->setDepthWriteEnabled(true);
  DepthState = _pDevice->newDepthStencilState(depthDesc);
  if(!DepthState)
  {
    std::fprintf(stderr, "failed to create depth state\n");
    Assert(false);
  }
  depthDesc->release();
}

Renderer::~Renderer()
{
  DepthState->release();
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
  pDesc->setDepthAttachmentPixelFormat(MTL::PixelFormat::PixelFormatDepth32Float);
  // pDesc->setFrontFacingWinding(MTL::Winding::WindingCounterClockwise);
  // pDesc->setCullMode(MTL::CullMode::CullModeBack);

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

  static float angle = 0.01f;
  angle += 0.01f;

  CGSize drawableSize = pView->drawableSize();
  float aspect = (float)drawableSize.width / (float)drawableSize.height;

  simd_float4x4 model = RotationY(angle);
  simd_float4x4 view = Translation(0.0f, 0.0f, -3.0f);
  simd_float4x4 projection = Perspective(1.0472f, aspect, 0.1f, 100.0f);
  simd_float4x4 mvp = simd_mul(projection, simd_mul(view, model));

  MTL::CommandBuffer* Command = CommandQueue->commandBuffer();
  MTL::RenderPassDescriptor* RenderPassDescriptor = pView->currentRenderPassDescriptor();
  if(RenderPassDescriptor)
  {
    MTL::RenderCommandEncoder* Encoder = Command->renderCommandEncoder( RenderPassDescriptor );
    Encoder->setDepthStencilState(DepthState);
    Encoder->setRenderPipelineState( _pPSO );
    Encoder->setVertexBuffer(VertexBuffer, 0, 0);
    Encoder->setVertexBytes(&mvp, sizeof(mvp), 1);
    Encoder->drawIndexedPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle, 36, MTL::IndexType::IndexTypeUInt16, IndexBuffer, 0);
    Encoder->endEncoding();

    Command->presentDrawable( pView->currentDrawable() );
    Command->commit();
  }

  pool->release();
}

void MyMTKViewDelegate::drawInMTKView( MTK::View* pView )
{
  _pRenderer->draw( pView );
}

static simd_float4x4 RotationY(float angle)
{
  float c = std::cos(angle);
  float s = std::sin(angle);

  simd_float4x4 m = matrix_identity_float4x4;

  m.columns[0] = simd_make_float4(c, 0, -s, 0);
  m.columns[1] = simd_make_float4(0, 1, 0, 0);
  m.columns[2] = simd_make_float4(s, 0, c, 0);

  return m;
}

static simd_float4x4 Translation(float x, float y, float z)
{
  simd_float4x4 m = matrix_identity_float4x4;
  m.columns[3] = simd_make_float4(x, y, z, 1);
  return m;
}

static simd_float4x4 Perspective(float fovY, float aspect, float nearZ, float farZ)
{
  float yScale = 1.0 / std::tan(fovY * 0.5);
  float xScale = yScale / aspect;

  simd_float4x4 m = {};
  m.columns[0] = simd_make_float4(xScale, 0, 0, 0);
  m.columns[1] = simd_make_float4(0, yScale, 0, 0);
  m.columns[2] = simd_make_float4(0, 0, farZ / (nearZ - farZ), -1);
  m.columns[3] = simd_make_float4(0, 0, (nearZ * farZ) / (nearZ - farZ), 0);
  return m;
}
