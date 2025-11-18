#version 400 core

in vec2 uv;
in vec3 cube_uv;

out vec4 out_frag_color;

uniform sampler2D s_position;
uniform sampler2D s_albedo;
uniform sampler2D s_normal;
uniform sampler2D s_effects;

uniform samplerCube ibl_convolution;
uniform samplerCube ibl_prefilter;
uniform sampler2D ibl_brdf_lut;

uniform samplerCube env_cube;

uniform vec3 view_pos;
uniform mat3 view_matrix3;
uniform mat3 inv_view_matrix3;
uniform mat4 view_matrix4;

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
vec4 direct_irradiance(vec4 radiance, vec3 albedo, vec3 V, vec3 N, vec3 L, vec3 F0, float roughness, float metalness)
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

struct DirectionLight
{
    vec3 direction;
    vec4 color;
};
uniform DirectionLight d_light;

struct PointLight
{
    vec3 position;
    vec4 color;
};
uniform PointLight p_light[4];

void main()
{
    // read gbuffer
    float depth = texture(s_position, uv).a;
    if (depth >= 0.99999) 
    {
        out_frag_color = texture(env_cube, cube_uv);
        return;
    }
    vec3 view_pos = texture(s_position, uv).rgb;
    vec3 albedo = pow(texture(s_albedo, uv).rgb, vec3(2.2));
    float roughness = texture(s_albedo, uv).a;
    vec3 normal = texture(s_normal, uv).xyz;
    float metalness = texture(s_normal, uv).a;
    float ao = texture(s_effects, uv).r;
    // direct light
    vec4 Lo = 0.0;
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metalness);    
    vec3 V = normalize(-view_pos);
    vec3 N = normalize(normal);
    // direction light
    vec3 L = normalize(view_matrix3 * (-d_light.direction));
    vec4 radiance = d_light.color;
    Lo += direct_irradiance(radiance, albedo, V, N, L, F0, roughness, metalness);
    // point light
    for (int i = 0; i < 4; i++)
    {
        vec4 light_pos = view_matrix4 * vec4(p_light[i].position, 1.0);
        L = normalize(light_pos.xyz - view_pos);
        float d = length(light_pos.xyz - view_pos);
        float attenuation = 1 / (d * d);
        radiance = attenuation * p_light[i].color;
        Lo += direct_irradiance(radiance, albedo, V, N, L, F0, roughness, metalness);
    }
    // IBL
    float NdotV = max(dot(N, V), 0.0);
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;
    vec3 N_world = inv_view_matrix3 * N;
    vec3 V_world = inv_view_matrix3 * V;
    // diffuse
    vec3 irradiance = texture(ibl_convolution, N_world).rgb;
    vec3 diffuse = kD * irradiance * albedo;
    // specular
    vec3 R = reflect(-V_world, N_world);
    vec3 prefilteredColor = textureLod(ibl_prefilter, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf = texture(ibl_brdf_lut, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    // mix
    vec3 ambient = (diffuse + specular) * ao;
    // final render equation
    vec3 final_color = ambient + Lo;
    // HDR tonemapping
    final_color = final_color / (final_color + vec3(1.0));
    // gamma correct
    final_color = pow(final_color, vec3(1.0/2.2)); 
    // out
    out_frag_color = vec4(final_color , 1.0);
}