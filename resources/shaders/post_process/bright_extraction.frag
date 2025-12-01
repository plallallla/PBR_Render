#version 400 core
out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTexture;

const vec3 luma = vec3(0.2126, 0.7152, 0.0722);

void main()
{
    vec3 color = texture(screenTexture, uv).rgb;
    if(dot(color, luma) > 1.0)
    {
        FragColor = vec4(color, 1.0);
    }
    else
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
} 