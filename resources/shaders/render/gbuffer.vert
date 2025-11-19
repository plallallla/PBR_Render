#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec2 vout_uv;
out vec3 vout_world_pos;
out vec3 vout_world_normal;
out vec4 vout_frag_position;
out vec4 vout_frag_prev_position;

uniform mat4 model;
uniform mat3 normal_matrix;
uniform mat4 proj_view_model;
uniform mat4 prev_proj_view_model;

void main()
{
    vout_uv = uv;
    vout_world_pos = (model * vec4(position, 1.0)).xyz;
    vout_world_normal = normal_matrix * normal;
    vout_frag_position = proj_view_model * vec4(position.xyz, 1.0);
    vout_frag_prev_position = prev_proj_view_model * vec4(position.xyz, 1.0);
    gl_Position = vout_frag_position;
}
