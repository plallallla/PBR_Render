#version 400 core

layout (location = 0) out vec4 g_position;
layout (location = 1) out vec4 g_albedo;
layout (location = 2) out vec4 g_normal;
layout (location = 3) out vec3 g_effects;

in vec2 vout_uv;
in vec3 vout_world_pos;
in vec3 vout_world_normal;
in vec4 vout_frag_position;
in vec4 vout_frag_prev_position;

const float near_plane = 0.1;
const float far_plane = 100.0;

uniform sampler2D s_albedo;
uniform sampler2D s_normal;
uniform sampler2D s_roughness;
uniform sampler2D s_metallic;
uniform sampler2D s_ao;

uniform vec3 eye_position;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(s_normal, vout_uv).xyz * 2.0 - 1.0;
    vec3 Q1 = dFdx(vout_world_pos);
    vec3 Q2 = dFdy(vout_world_pos);
    vec2 st1 = dFdx(vout_uv);
    vec2 st2 = dFdy(vout_uv);
    vec3 N = normalize(vout_world_normal);
    vec3 T = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

vec2 normalize_frag_pos(vec4 pos)
{
    return pos.xy / pos.w * 0.5 + 0.5;
}

void main()
{
    // g_position : position + depth
    g_position.xyz = vout_world_pos;
    float depth = length(eye_position - vout_world_pos);
    g_position.w = (depth - near_plane) / (far_plane - near_plane);
    // g_albedo : albedo + roughness
    g_albedo.rgb = texture(s_albedo, vout_uv).rgb;
    g_albedo.a = texture(s_roughness, vout_uv).r;
    // g_normal : normal + metalic
    g_normal.rgb = getNormalFromMap();
    g_normal.a = texture(s_metallic, vout_uv).r;
    // g_effects : ao + motion vector
    g_effects.r = texture(s_ao, vout_uv).r;
    g_effects.gb = normalize_frag_pos(vout_frag_position) - normalize_frag_pos(vout_frag_prev_position);
}