#version 460

layout (set=0, binding=0) uniform CameraBuffer
{
    mat4 viewMatrix;
};

layout (set=0, binding=1) uniform SpriteSheetInfo
{
    vec2 spriteSize;
};

layout (push_constant) uniform Contants
{
    mat4 modelMatrix;
    uvec2 spriteCoords;
};

layout (location = 0) out vec2 texCoords;

vec2 positions[] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),

    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
};

void main() {
    gl_Position = viewMatrix * modelMatrix * vec4(positions[gl_VertexIndex].xy, 0.0, 1.0);
    texCoords = spriteSize * (spriteCoords + positions[gl_VertexIndex].xy);
}
