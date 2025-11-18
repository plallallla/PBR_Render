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

vec3 transform_normal()
{
    vec3 dPdx  = dFdx(vout_view_pos);
    vec3 dPdy  = dFdy(vout_view_pos);
    vec2 dUVdx = dFdx(vout_uv);
    vec2 dUVdy = dFdy(vout_uv);
    vec3 tangent = vec3(1, 0, 0); 
    vec3 binormal = vec3(0, 1, 0);
    float det = dUVdx.x * dUVdy.y - dUVdx.y * dUVdy.x;
    if (abs(det) > 1e-6)
    {
        tangent = (dPdx * dUVdy.y - dPdy * dUVdx.y) / det;
        binormal = (dPdy * dUVdx.x - dPdx * dUVdy.x) / det;
        if (det < 0.0) binormal = -binormal;
    }
    uv_normal.g = -uv_normal.g;//DX3D style
    vec3 uv_normal = normalize(texture(s_normal, vout_uv).rgb * 2.0f - 1.0f);
    vec3 normal = normalize(vout_normal);        
    mat3 TBN = mat3(normalize(tangent), normalize(binormal), normal);
    return normalize(TBN * uv_normal);
}

vec2 normalize_frag_pos(vec4 pos)
{
    return pos.xy / pos.w * 0.5 + 0.5;
}

void main()
{
    // g_position : position + depth
    g_position.xyz = vout_view_pos;
    g_position.w = -vout_view_pos.z;
    // g_albedo : albedo + roughness
    g_albedo.rgb = texture(s_albedo, vout_uv).rgb;
    g_albedo.a = texture(s_roughness, vout_uv).r;
    // g_normal : normal + metalness
    g_normal.rgb = transform_normal();
    g_normal.a = texture(s_metalness, vout_uv).r;
    // g_effects : ao + motion vector
    g_effects.r = texture(s_ao, vout_uv).r;
    g_effects.gb = normalize_frag_pos(vout_frag_position) - normalize_frag_pos(vout_frag_prev_position);
}