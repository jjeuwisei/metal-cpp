#include <metal_stdlib>
using namespace metal;

struct VertexOut
{
  float4 position [[position]];
  half3 color;
};

struct Vertex
{
  float4 position;
  float4 color;
};

vertex VertexOut vertexMain
( uint vertexId [[vertex_id]], device const Vertex* vertices[[buffer(0)]] ) 
{
  VertexOut attributes;
  attributes.position = vertices[vertexId].position;
  attributes.color = half3(vertices[vertexId].color.rgb);
  return attributes;
}

fragment half4 fragmentMain( VertexOut in [[stage_in]] )
{
  return half4( in.color, 1.0 );
}
