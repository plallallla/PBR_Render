#version 400 core

out vec2 uv;
out vec3 cube_uv;

uniform mat4 inverse_view;
uniform mat4 inverse_projection;

void main()
{
    uv = vec2((gl_VertexID & 1) << 2, (gl_VertexID & 2) << 1);
    vec4 position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
    vec4 world_pos = inverse_view * (inverse_projection * position);
    cube_uv = world_pos.xyz / world_pos.w;
    gl_Position = position;
}