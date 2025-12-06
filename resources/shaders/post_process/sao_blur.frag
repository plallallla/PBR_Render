#version 400 core

out float o_sao_blur;
in vec2 uv;

uniform sampler2D screenTexture;
uniform vec2 frag_size;
uniform int blur_size;

void main()
{
   float result = 0.0;
   for (int x = 0; x < blur_size; ++x)
   {
      for (int y = 0; y < blur_size; ++y)
      {
        vec2 offset = (vec2(-2.0) + vec2(float(x), float(y))) * frag_size;
        result += texture(screenTexture, uv + offset).r;
      }
   }

   o_sao_blur = result / float(blur_size * blur_size);
}