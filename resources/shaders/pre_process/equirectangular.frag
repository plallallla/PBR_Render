#version 400 core

out vec4 fout_color;
in vec3 vout_world_position;

uniform sampler2D equirectangular_map;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(vout_world_position));
    fout_color = pow(texture(equirectangular_map, uv), vec4(2.2));
}
