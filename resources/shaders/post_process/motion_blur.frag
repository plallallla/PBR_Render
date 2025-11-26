#version 400 core

out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTexture;
uniform sampler2D gEffects;

const int max_samples = 16;
const float blur_scale = 1.0;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(screenTexture, 0));
    vec2 velocity = texture(gEffects, uv).gb * blur_scale;
    velocity = clamp(velocity, -vec2(10.0), vec2(10.0));//限制最大速度
    float frag_speed = length(velocity / texel_size);
    int num_samples = clamp(int(frag_speed), 1, max_samples);

    if (num_samples <= 1) 
    {
        FragColor = texture(screenTexture, uv);
        return;
    }

    vec3 color_sum = vec3(0.0);
    for (int i = 0; i < num_samples; ++i) 
    {
        float t = float(i) / float(num_samples - 1);
        vec2 offset = velocity * (t - 0.5);
        color_sum += texture(screenTexture, uv + offset).rgb;
    }

    FragColor = vec4(color_sum / float(num_samples), 1.0);
}