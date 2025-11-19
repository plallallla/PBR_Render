#version 400 core

layout (location = 0) out vec4 g_position;
layout (location = 1) out vec4 g_normal;

in vec2 vout_uv;
in vec3 vout_world_pos;
in vec3 vout_world_normal;

void main()
{
    g_position.xyz = vout_world_pos;
    g_normal.xyz = vout_world_normal;
}
