#include <metal_stdlib>
using namespace metal;

struct VertexOut
{
  float4 position [[position]];
  half3 color;
};

struct VertexIn
{
  float4 position [[attribute(0)]];
  float4 color [[attribute(1)]];
};

vertex VertexOut vertexMain
( VertexIn in [[stage_in]], constant float4x4& mvp[[buffer(1)]]) 
{
  VertexOut attributes;
  attributes.position = mvp * in.position;
  attributes.color = half3(in.color.rgb);
  return attributes;
}

fragment half4 fragmentMain( VertexOut in [[stage_in]] )
{
  return half4( in.color, 1.0 );
}
