#version 400 core
out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTexture1;
uniform sampler2D screenTexture2;

void main()
{
    vec4 color1 = vec4(texture(screenTexture1, uv).rgb, 1.0);
    vec4 color2 = vec4(texture(screenTexture2, uv).rgb, 1.0);
    FragColor = color1 + color2 * 0.5;
} 