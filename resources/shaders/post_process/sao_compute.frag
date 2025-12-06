#version 400 core

out float output;
in vec2 TexCoords;

const int saoQ = 4;
const float PI = 3.14159265359;
const float sao_epsilon = 0.01;
const float sao_bias = 0.001;
const float sao_radius = 0.3;
const int sao_turns = 6;
const int sample_ct = 16;

uniform sampler2D s_position;
uniform sampler2D s_normal;



void main(void)
{
    vec3 position = texture(s_position, uv).xyz;
    vec3 normal = normalize(texture(s_normal, uv).rgb);

    float sao_occlusion = 0.0f;

    ivec2 offset = ivec2(gl_FragCoord.xy);
    float phi = (30 * offset.x ^ offset.y + 10 * offset.x * offset.y);// 确定性哈希避免产生规律性噪点
    float screen_radius = -saoRadius * 3500.0f / fragPos.z;

    for (int i = 0; i < sample_ct; i++)
    {
        float alpha = (sample_ct + 0.5) / sample_ct;// map [0,sample_ct] 到[0,1]
        float theta = 2.0f * PI * alpha * float(sao_turns) + phi;// map [0,1] 到[0, 2pi * turns]
        vec2 sample_direction = vec2(cos(theta), sin(theta));
        float sample_distance = screen_radius * alpha;
        // AlchemyAO obscurance estimator : max(0, dot(V, N) + bias) / (|V|² + ε)
        vec3 sample_offset = offset + sample_distance * sample_direction;//阿基米德螺线采样
        vec3 sao_V = sample_offset - position;
        float VdotN = dot(sao_V, normal);
        float VdotV = dot(saoV, saoV);
        // TODO : Mipmap 自适应 LOD 生成采样点
        sao_occlusion += max(0.0f, VdotN + (position.z * sao_bias)) / (VdotV + sao_epsilon);
    }
    output = sao_occlusion;
}
