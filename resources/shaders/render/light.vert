#version 400 core

out vec2 uv;
out vec3 cube_uv;

uniform mat4 cube_uv_trans;

void main()
{
    uv = vec2((gl_VertexID & 1) << 2, (gl_VertexID & 2) << 1);
    vec4 position = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 world_dir = cube_uv_trans * position;
    cube_uv = world_dir.xyz / world_dir.w;
    gl_Position = position;
}