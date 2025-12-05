#version 400 core

out float output;
in vec2 TexCoords;

const int saoQ = 4;
const float PI = 3.14159265359;
const float sao_epsilon = 0.01;
const float sao_bias = 0.001;
const int sample_ct = 8;

uniform sampler2D s_position;
uniform sampler2D s_normal;



void main(void)
{
    vec3 position = texture(s_position, uv).xyz;
    vec3 normal = normalize(texture(s_normal, uv).rgb);

    float sao_occlusion = 0.0f;

    ivec2 offset = ivec2(gl_FragCoord.xy);
    float saoPhi = (30 * offset.x ^ offset.y + 10 * offset.x * offset.y);


    for (int i = 0; i < sample_ct; i++)
    {
        // AlchemyAO obscurance estimator
        // max(0, dot(V, N) + bias) / (|V|² + ε)
        vec3 sample_offset;//TODO:确定性螺旋采样 + Mipmap 自适应 LOD 生成采样点
        vec3 sao_V = sample_offset - position;
        float VdotN = dot(sao_V, normal);
        float VdotV = dot(saoV, saoV);

        sao_occlusion += max(0.0f, VdotN + (position.z * sao_bias)) / (VdotV + sao_epsilon);
    }
    output = sao_occlusion;
}
