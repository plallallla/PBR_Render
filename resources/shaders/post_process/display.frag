#version 400 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D screenTexture;

void main()
{
    FragColor = vec4(texture(screenTexture, uv).rgb, 1.0);
} 