#version 400 core

out vec4 o_sao_blur;
in vec2 uv;

uniform sampler2D screenTexture;
uniform sampler2D sao_result;
uniform vec2 frag_size;

const int blur_size = 4;

void main()
{
   o_sao_blur = vec4(vec3(texture(sao_result, uv).r), 1.0);
   return;
   // vec3 result = texture(screenTexture, uv).rgb;
   // float sao = 0.0;
   // for (int x = 0; x < blur_size; ++x)
   // {
   //    for (int y = 0; y < blur_size; ++y)
   //    {
   //      vec2 offset = (vec2(-2.0) + vec2(float(x), float(y))) * frag_size;
   //      sao += texture(sao_result, uv + offset).r;
   //    }
   // }
   // result * = sao / float(blur_size * blur_size);
   // o_sao_blur = vec4(result, 1.0);
}