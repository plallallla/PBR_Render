#version 400 core

out vec4 o_color;
in vec2 uv;

uniform sampler2D input_texture;

void main()
{
   o_color = vec4(vec3(texture(input_texture, uv).r), 1.0);
}