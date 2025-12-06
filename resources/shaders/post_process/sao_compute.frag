#version 400 core

out float o_sao;
in vec2 uv;

const int saoQ = 4;
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
    float depth = texture(s_position, uv).w;
    vec3 normal = normalize(texture(s_normal, uv).rgb);
    ivec2 offset = ivec2(gl_FragCoord.xy);
    float phi = (30 * offset.x ^ offset.y + 10 * offset.x * offset.y);// 确定性哈希避免产生规律性噪点
    float screen_radius = 15;
    for (int i = 0; i < sample_ct; i++)
    {
        // 确定当前阿基米德螺线采样点
        float alpha = (i + 0.5) / sample_ct;// map [0,sample_ct] 到[0,1]
        float theta = 2.0 * PI * float(sao_turns) * alpha + phi;// map [0,1] 到[0, 2pi * turns]
        vec2 sample_direction = vec2(cos(theta), sin(theta));
        float sample_distance = screen_radius * alpha;
        // Mipmap自适应LOD生成采样点
        int mipmap_level = clamp(findMSB(int(sample_distance)) - 4, 0, max_mipmap_level);
        vec3 sample_pt = textureLod(s_position, uv + sample_direction * screen_radius * alpha, mipmap_level).xyz;
        vec3 sao_V = sample_pt - position;
        float VdotN = dot(sao_V, normal);
        float VdotV = dot(sao_V, sao_V);
        // AlchemyAO obscurance estimator : max(0, dot(V, N) + bias) / (|V|² + ε)
        sao_occlusion += max(0.0f, VdotN + (depth * sao_bias)) / (VdotV + sao_epsilon);
    }
    o_sao = sao_occlusion;
}
