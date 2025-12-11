#version 400 core

out float o_sao;
in vec2 uv;

uniform sampler2D sao_compute_result;
uniform vec2 frag_size;

const int blur_size = 4;

void main()
{
   float sao = 0.0;
   for (int x = 0; x < blur_size; ++x)
   {
      for (int y = 0; y < blur_size; ++y)
      {
        vec2 offset = (vec2(-2.0) + vec2(float(x), float(y))) * frag_size;
        sao += texture(sao_compute_result, uv + offset).r;
      }
   }
   sao /= float(blur_size * blur_size);
   o_sao = sao;
}