#version 400 core

layout (location = 0) out vec4 g_position;
layout (location = 1) out vec4 g_albedo;
layout (location = 2) out vec4 g_normal;
layout (location = 3) out vec3 g_effects;

in vec3 vout_view_pos;
in vec2 vout_uv;
in vec3 vout_normal;
in vec4 vout_frag_position;
in vec4 vout_frag_prev_position;

const float near_plane = 1.0f;
const float far_plane = 1000.0f;

uniform sampler2D s_albedo;
uniform sampler2D s_normal;
uniform sampler2D s_roughness;
uniform sampler2D s_metalness;
uniform sampler2D s_ao;

vec3 trans_normal(vec3 view_normal, vec3 uv_normal)
{
    vec3 dPosX  = dFdx(vout_view_pos);
    vec3 dPosY  = dFdy(vout_view_pos);
    vec2 dTexX = dFdx(vout_uv);
    vec2 dTexY = dFdy(vout_uv);

    vec3 normal = normalize(view_normal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * uv_normal);
}

vec2 normalize_frag_pos(vec4 pos)
{
    return pos.xy / pos.w * 0.5 + 0.5;
}

void main()
{
    // g_position : position + depth
    float z = vout_view_pos.z;
    z = (z-0.1)/(100.0-0.1);
    g_position = vec4(z,z,z,1.0);
    // g_position.xyz = vout_view_pos;
    // g_position.w = -vout_view_pos.z;
    // g_albedo : albedo + roughness
    g_albedo.rgb = texture(s_albedo, vout_uv).rgb;
    g_albedo.a = texture(s_roughness, vout_uv).r;
    // g_normal : normal + metalness
    vec3 uv_normal = normalize(texture(s_normal, vout_uv).rgb * 2.0f - 1.0f);
    uv_normal.g = -uv_normal.g;//DX3D
    g_normal.rgb = trans_normal(vout_normal, uv_normal);
    g_normal.a = texture(s_metalness, vout_uv).r;
    // g_effects : ao + motion vector
    g_effects.r = texture(s_ao, vout_uv).r;
    g_effects.gb = normalize_frag_pos(vout_frag_position) - normalize_frag_pos(vout_frag_prev_position);
}