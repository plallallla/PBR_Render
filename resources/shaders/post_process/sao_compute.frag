#version 400 core

out float o_sao;
in vec2 uv;

const float PI = 3.14159265359;
const float sao_epsilon = 0.01;
const float sao_bias = 0.001;
const float sao_radius = 0.3;
const int sao_turns = 6;
const int sample_ct = 16;

uniform sampler2D s_position;
uniform sampler2D s_normal;
uniform int max_mipmap_level;

void main(void)
{
    float sao_occlusion = 0.0;
    vec3 position = texture(s_position, uv).xyz;
    vec3 normal = texture(s_normal, uv).xyz;
    ivec2 offset = ivec2(gl_FragCoord.xy);
    float screen_radius = 10.0;
    for (int i = 0; i < sample_ct; i++)
    {
        // 确定当前阿基米德螺线采样点
        float alpha = (i + 0.5) / sample_ct;// map [0,sample_ct] 到[0,1]
        // float theta = 2.0 * PI * float(sao_turns) * alpha + phi;// map [0,1] 到[0, 2pi * turns]
        float theta = 2.0 * PI * float(sao_turns) * alpha;// map [0,1] 到[0, 2pi * turns]
        vec2 sample_direction = vec2(cos(theta), sin(theta));
        float sample_distance = screen_radius * alpha;
        // Mipmap自适应LOD生成采样点
        int mipmap_level = clamp(findMSB(int(sample_distance)) - 4, 0, max_mipmap_level);
        vec2 uv_offset = sample_direction * sample_distance >> mipmap_level;
        vec3 sample_pt = texture(s_position, uv + uv_offset).xyz;
        vec3 sao_V = sample_pt - position;
        float VdotN = dot(sao_V, normal) + sao_bias;
        float VdotV = dot(sao_V, sao_V);
        // AlchemyAO obscurance estimator : max(0, dot(V, N) + bias) / (|V|² + ε)
        sao_occlusion += max(0.0, VdotN) / (VdotV + sao_epsilon);
    }
    o_sao = sao_occlusion;
}
