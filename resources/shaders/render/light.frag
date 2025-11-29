#version 400 core

in vec2 uv;
in vec3 cube_uv;

out vec4 fragment_color;

uniform sampler2D s_position;
uniform sampler2D s_albedo;
uniform sampler2D s_normal;
uniform sampler2D s_effects;
uniform samplerCube ibl_convolution;
uniform samplerCube ibl_prefilter;
uniform sampler2D ibl_brdf_lut;
uniform samplerCube env_cube;

uniform vec3 eye_position;

struct DirectionLight
{
    vec3 direction;
    vec3 color;
};
uniform DirectionLight d_light;
uniform sampler2D d_shadow_text;
uniform mat4 d_light_matrix;

uniform vec2 fragment_size;

struct PointLight
{
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};
uniform PointLight p_light[4];

const float near_plane = 0.1;
const float far_plane = 75.0;
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

float directional_shadow(vec3 world_pos, float bias)
{
    vec4 frag_light_pos = d_light_matrix * vec4(world_pos, 1.0);
    frag_light_pos = frag_light_pos / frag_light_pos.w;
    frag_light_pos = frag_light_pos * 0.5 + 0.5;
    // 裁剪空间外的点不受阴影影响（或视为不在阴影中）
    if (frag_light_pos.z > 1.0 || frag_light_pos.x < 0.0 || frag_light_pos.x > 1.0 || frag_light_pos.y < 0.0 || frag_light_pos.y > 1.0)
    {
        return 1.0; // 无阴影
    }
    float shadow = 0.0;
    for (int x = -1; x < 2; x++)
    {
        for (int y = -1; y < 2; y++)
        {
            vec2 offset = vec2(x, y) * fragment_size;
            float pcf_depth = texture(d_shadow_text, frag_light_pos.xy + offset).r;
            shadow += frag_light_pos.z > pcf_depth + bias ? 1.0 : 0.0;
        }
    }
    return 1.0 - (shadow / 9);
}

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH2 = NdotH*NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 direct_irradiance(vec3 radiance, vec3 albedo, vec3 V, vec3 N, vec3 L, vec3 F0, float roughness, float metalness)
{
    vec3 H = normalize(V + L);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    // specular
    float NDF = DistributionGGX(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    vec3 F = fresnelSchlick(HdotV, F0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;
    // diffuse
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metalness);
    vec3 diffuse = kD * albedo / PI;
    // mix
    return (diffuse + specular) * radiance * NdotL;
}

vec3 indirect_irradiance(vec3 N, vec3 V, vec3 albedo, vec3 F0, float roughness, float metallic, float ao)
{
    float NdotV = max(dot(N, V), 0.0);
    vec3 R = reflect(-V, N); 
    vec3 F_env = fresnelSchlickRoughness(NdotV, F0, roughness);
    // diffuse
    vec3 convolution = texture(ibl_convolution, N).rgb;
    vec3 diffuse = (1.0 - F_env) * (1 - metallic) * convolution * albedo;
    // specular
    vec3 prefilter = textureLod(ibl_prefilter, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(ibl_brdf_lut, vec2(NdotV, roughness)).xy;
    vec3 specular = (F_env * brdf.x + brdf.y) * ao * prefilter;
    return diffuse + specular;
}

void main()
{
    // skybox
    float depth = texture(s_position, uv).w;
    if (depth == 1) 
    {
        fragment_color = texture(env_cube, cube_uv);
        return;
    }
    // read gbuffer
    vec3 world_space_position = texture(s_position, uv).rgb;
    vec3 normal = texture(s_normal, uv).xyz;
    vec3 albedo = texture(s_albedo, uv).rgb;
    float roughness = texture(s_albedo, uv).a;
    float metallic = texture(s_normal, uv).a;
    // input lighting data
    vec3 N = normalize(normal);
    vec3 V = normalize(eye_position - world_space_position);
    vec3 R = reflect(-V, N); 
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);
    // directional light
    vec3 L = normalize(d_light.direction);
    vec3 H = normalize(V + L);
    vec3 radiance = d_light.color;
    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
    float visibility = directional_shadow(world_space_position, bias);
    Lo += direct_irradiance(radiance, albedo, V, N, L, F0, roughness, metallic) * visibility;
    // point light
    for (int i = 0; i < 4; i++)
    {
        L = normalize(p_light[i].position - world_space_position);
        H = normalize(V + L);
        float d = distance(p_light[i].position, world_space_position);
        float attenuation = 1.0 / (p_light[i].constant + d * p_light[i].linear + d * d * p_light[i].quadratic);
        radiance = p_light[i].color * attenuation;
        // Lo += direct_irradiance(radiance, albedo, V, N, L, F0, roughness, metallic);
    }
    // ibl
    float ao = texture(s_effects, uv).r;
    vec3 ibl = indirect_irradiance(N, V, albedo, F0, roughness, metallic, ao);
    // mix
    fragment_color = vec4(Lo + ibl, 1.0);
    return;
}
