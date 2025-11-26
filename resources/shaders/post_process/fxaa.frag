#version 330 core

uniform sampler2D screenTexture;
uniform vec2 frag_size;

in vec2 uv;
out vec4 frag_color;

float CalculateLuma(vec3 rgb) 
{
    return rgb.r * 0.299 + rgb.g * 0.587 + rgb.b * 0.114;
}

const float edge_threshold = 1.0 / 8.0;
const float span_max = 8.0;
const float reduce_mul = 1.0 / 8.0;
const float reduce_min = 1.0 / 128.0;

void main() 
{
    vec3 rgbN = texture(screenTexture, uv - frag_size * vec2(0, 1)).rgb;
    vec3 rgbS = texture(screenTexture, uv + frag_size * vec2(0, 1)).rgb;
    vec3 rgbW = texture(screenTexture, uv - frag_size * vec2(1, 0)).rgb;
    vec3 rgbE = texture(screenTexture, uv + frag_size * vec2(1, 0)).rgb;
    vec3 rgbM = texture(screenTexture, uv).rgb;

    float lumaN = CalculateLuma(rgbN);
    float lumaE = CalculateLuma(rgbE);
    float lumaS = CalculateLuma(rgbS);
    float lumaW = CalculateLuma(rgbW);
    float lumaM = CalculateLuma(rgbM);

    float max_luma = max(max(max(lumaN, lumaE), max(lumaS, lumaW)), lumaM);
    float min_luma = min(min(min(lumaN, lumaE), min(lumaS, lumaW)), lumaM);
    float range_luma = max_luma - min_luma;

    if (range_luma < max(edge_threshold, max_luma * 0.0625))
    {
        frag_color = vec4(rgbM, 1.0);
        return;
    }

    // 计算方向
    float gradX = lumaE - lumaW;   // 水平梯度
    float gradY = lumaS - lumaN;   // 垂直梯度
    float dirReduce = max((lumaN + lumaS + lumaE + lumaW) * (0.25 * reduce_mul), reduce_min);
    float rcpDirMin = 1.0 / (min(abs(gradX), abs(gradY)) + dirReduce);

    vec2 dir = vec2(gradX, gradY);
    dir = clamp(dir * rcpDirMin, -vec2(span_max), vec2(span_max));
    dir *= frag_size;

    // 计算混合值
    vec3 rgbA = 0.5 * (texture(screenTexture, uv + dir * (1.0 / 3.0 - 0.5)).rgb +
    texture(screenTexture, uv + dir * (2.0 / 3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(screenTexture, uv + dir * (0.0 / 3.0 - 0.5)).rgb +
    texture(screenTexture, uv + dir * (3.0 / 3.0 - 0.5)).rgb);
    float lumaB = CalculateLuma(rgbB);
    if ((lumaB < min_luma) || (lumaB > max_luma))
    {
        frag_color = vec4(rgbA, 1.0);
    }
    else
    {
        frag_color = vec4(rgbB, 1.0);
    }
}