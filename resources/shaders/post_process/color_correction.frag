#version 400 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D screenTexture;
void main()
{
    vec4 color = vec4(texture(screenTexture, uv).rgb, 1.0);
    color = color / (color + vec4(1.0));
    color = pow(color, vec4(1.0/2.2));
    FragColor = color;
} 