#version 400 core

layout (location = 0) in vec3 position;

out vec3 vout_world_position;

uniform mat4 projection_view;

void main()
{
    vout_world_position = position;
    gl_Position = projection_view * vec4(position.xyz, 1.0);
}
