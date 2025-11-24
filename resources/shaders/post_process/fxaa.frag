#version 400 core
out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTexture;
uniform vec2 texelSize;

const vec3 LUMA = vec3(0.299, 0.587, 0.114);
const float FXAA_REDUCE_MUL = 1.0/8.0;
const float FXAA_REDUCE_MIN = 1.0/128.0;
const float FXAA_SPAN_MAX = 8.0;



vec3 computeFxaa()
{
    vec2 screenTextureOffset = screenTexture;
    vec3 luma = vec3(0.299, 0.587, 0.114);

    vec3 offsetNW = texture(screenTexture, uv.xy + (vec2(-1.0f, -1.0f) * screenTextureOffset)).xyz;
    vec3 offsetNE = texture(screenTexture, uv.xy + (vec2(1.0f, -1.0f) * screenTextureOffset)).xyz;
    vec3 offsetSW = texture(screenTexture, uv.xy + (vec2(-1.0f, 1.0f) * screenTextureOffset)).xyz;
    vec3 offsetSE = texture(screenTexture, uv.xy + (vec2(1.0f, 1.0f) * screenTextureOffset)).xyz;
    vec3 offsetM  = texture(screenTexture, uv.xy).xyz;

    float lumaNW = dot(luma, offsetNW);
    float lumaNE = dot(luma, offsetNE);
    float lumaSW = dot(luma, offsetSW);
    float lumaSE = dot(luma, offsetSE);
    float lumaM  = dot(luma, offsetNW);

    vec2 dir = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)),
                     ((lumaNW + lumaSW) - (lumaNE + lumaSE)));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (FXAA_REDUCE_MUL * 0.25f), FXAA_REDUCE_MIN);
    float dirCorrection = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * dirCorrection)) * screenTextureOffset;

    vec3 resultA = 0.5f * (texture(screenTexture, uv.xy + (dir * vec2(1.0f / 3.0f - 0.5f))).xyz +
                                    texture(screenTexture, uv.xy + (dir * vec2(2.0f / 3.0f - 0.5f))).xyz);

    vec3 resultB = resultA * 0.5f + 0.25f * (texture(screenTexture, uv.xy + (dir * vec2(0.0f / 3.0f - 0.5f))).xyz +
                                             texture(screenTexture, uv.xy + (dir * vec2(3.0f / 3.0f - 0.5f))).xyz);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float lumaResultB = dot(luma, resultB);

    if(lumaResultB < lumaMin || lumaResultB > lumaMax)
        return vec3(resultA);
    else
        return vec3(resultB);
}


void main() 
{
    vec3 rgbNW = texture(screenTexture, uv + texelSize * vec2(-1,-1)).rgb;
    vec3 rgbNE = texture(screenTexture, uv + texelSize * vec2(1,-1)).rgb;
    vec3 rgbSW = texture(screenTexture, uv + texelSize * vec2(-1,1)).rgb;
    vec3 rgbSE = texture(screenTexture, uv + texelSize * vec2(1,1)).rgb;
    vec3 rgbM  = texture(screenTexture, uv).rgb;

    float lumaNW = dot(rgbNW, LUMA);
    float lumaNE = dot(rgbNE, LUMA);
    float lumaSW = dot(rgbSW, LUMA);
    float lumaSE = dot(rgbSE, LUMA);
    float lumaM  = dot(rgbM, LUMA);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    if (lumaMax - lumaMin < 0.0312) 
    {
        FragColor = vec4(rgbM, 1.0);
        return;
    }

    vec2 dir = vec2(
        -(lumaNW + lumaNE - lumaSW - lumaSE),
         (lumaNW + lumaSW - lumaNE - lumaSE)
    );
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -FXAA_SPAN_MAX, FXAA_SPAN_MAX) * texelSize;

    vec3 result = (1.0/2.0) * 
    (
        texture(screenTexture, uv + dir * (1.0/3.0 - 0.5)).rgb +
        texture(screenTexture, uv + dir * (2.0/3.0 - 0.5)).rgb
    );
    // 临时输出颜色校正
    result = computeFxaa();
    // result = result / (result + vec3(1.0));
    // result = pow(result, vec3(1.0/2.2));
    FragColor = vec4(result, 1.0);

}

