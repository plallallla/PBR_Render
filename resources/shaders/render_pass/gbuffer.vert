#version core 400

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 uv;

out vec3 vout_view_pos;
out vec2 vout_uv;
out vec3 vout_normal;
out vec4 vout_frag_position;
out vec4 vout_frag_prev_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix;
uniform mat4 proj_view_model;
uniform mat4 prev_proj_view_model;

void main()
{
    vout_view_pos = projection * view * model * vec4(position.xyz, 1.0);
    vout_uv = uv;
    vout_normal = normal_matrix * normal;
    vout_frag_position = proj_view_model * vec4(position.xyz, 1.0);
    vout_frag_prev_position = prev_proj_view_model * vec4(position.xyz, 1.0);
}
