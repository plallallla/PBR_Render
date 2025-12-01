#version 400 core

uniform vec3 light_color;

out vec4 o_color;

void main()
{
    o_color = vec4(1.0, 0.0, 0.0, 1.0);
    // o_color = vec4(light_color, 1.0);
}