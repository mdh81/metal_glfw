#include <metal_stdlib>
using namespace metal;

#include "ShaderTypes.h"

struct VertexData {
    float4 position [[position]];
    float4 color;
};

vertex VertexData
processVertex(uint vertexId [[ vertex_id ]],
              constant VertexPositionAndColor* positionAndColor) {
    VertexData vertexOutput;
    vertexOutput.position = float4(positionAndColor[vertexId].position.xyz, 1.f);
    vertexOutput.color = float4(positionAndColor[vertexId].color.xyz, 1.f);
    return vertexOutput;
}

fragment float4 processFragment(VertexData vertexData [[stage_in]]) {
    return vertexData.color;
}