// reference from : https://github.com/timurson/AlchemyAO
#version 400 core

out float o_sao;
in vec2 uv;

const float PI = 3.14159265359;
const float TAU = 6.2831853072;
const float sao_epsilon = 0.01;
const float sao_bias = 0.001;
const float sao_radius = 100.0;
const float sao_intensity = 1.3;
const float sao_contrast = 0.5;
const int sample_turns = 6;
const int sample_ct = 16;
const int max_mip = 8;

uniform sampler2D s_position;
uniform sampler2D s_normal;
uniform mat4 position_transform;
uniform mat3 normal_transform;

// float randAngle()
// {
// 	uint x = uint(gl_FragCoord.x);
// 	uint y = uint(gl_FragCoord.y);
// 	return (30u * x ^ y + 10u * x * y);
// }

vec3 get_sample_pt(float radius, int i)
{
    float alpha = 1.0 / sample_ct * (i + 0.5);
    float h = radius * alpha;
    float theta = TAU * alpha * sample_turns;
    // float theta = TAU * alpha * sample_turns + randAngle();
	vec2 u = vec2(cos(theta), sin(theta));
	// McGuire paper MIP calculation
	int m = clamp(findMSB(int(h)) - 4, 0, max_mip);
	ivec2 mip_pos = clamp((ivec2(h * u) + ivec2(gl_FragCoord.xy)) >> m, ivec2(0), textureSize(s_position, m) - ivec2(1));
	vec3 worldPi = texelFetch(s_position, mip_pos, m).xyz;
	vec3 Pi = vec3(position_transform * vec4(worldPi, 1.0));
    return Pi;
}

void main(void)
{
    float sao_occlusion = 0.0;
	vec3 P = vec3(position_transform * vec4(texture(s_position, uv).xyz, 1.0));
    vec3 N = normalize(normal_transform * texture(s_normal, uv).xyz);
    float radius = -1.0 * sao_radius / P.z;
    for (int i = 0; i < sample_ct; i++)
    {
        vec3 sample_pt = get_sample_pt(radius, i);
        vec3 V = sample_pt - P;
        float VdotN = dot(V, N) + sao_bias;
        float VdotV = dot(V, V);
        // AlchemyAO obscurance estimator : max(0, dot(V, N) + bias) / (|V|² + ε)
        sao_occlusion += max(0.0, VdotN) / (VdotV + sao_epsilon);
    }
    float occlusion = (2.0 * sao_intensity * sao_occlusion) / float(sample_ct);
    float ao = 1.0 - pow(clamp(occlusion, 0.0, 1.0), sao_contrast);
    o_sao = clamp(ao, 0.0, 1.0);
}

