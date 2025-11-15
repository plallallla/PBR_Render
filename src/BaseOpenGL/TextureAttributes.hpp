#pragma once
#include <glad/glad.h>

struct TextureFilterMode
{
    GLuint _min = GL_LINEAR;
    GLuint _max = GL_LINEAR;
};

struct TextureWrapMode
{
    GLuint _s = GL_REPEAT;
    GLuint _t = GL_REPEAT;
    GLuint _r = GL_REPEAT;
};

struct TextureFormat
{
    GLuint _internal = GL_RGB8;
    GLuint _image    = GL_RGB;
    GLuint _pixel    = GL_UNSIGNED_BYTE;
};

struct TextureAttributes
{
    GLuint _target{ GL_TEXTURE_2D };
    TextureWrapMode _wrap;
    TextureFilterMode _filter;
    TextureFormat _format;
    bool _mipmap{ true };
};

inline auto TEXTURE_2D_RGB = [] () -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_REPEAT, GL_REPEAT,},
        {GL_LINEAR, GL_LINEAR,},
        {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},
        true
    };
}();

inline auto TEXTURE_2D_RGBA16F = [] () -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_REPEAT, GL_REPEAT}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE}, 
        true
    };
}();

inline auto TEXTURE_2D_SRGB = [] () -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_REPEAT, GL_REPEAT}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_SRGB8, GL_SRGB, GL_UNSIGNED_BYTE}, 
        true
    };
}();

inline auto TEXTURE_2D_GAMMA = [] () -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_REPEAT, GL_REPEAT}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE}, 
        true
    };
}();

inline auto TEXTURE_2D_GAMMA_ALPHA = [] () -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_REPEAT, GL_REPEAT}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_SRGB_ALPHA, GL_RGBA, GL_UNSIGNED_BYTE}, 
        true
    };
}();

inline auto TEXTURE_2D_RGBA = [] () -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_REPEAT, GL_REPEAT}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 
        true
    };
}();

inline auto TEXTURE_2D_GRAY = []() -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_REPEAT, GL_REPEAT}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_R8, GL_RED, GL_UNSIGNED_BYTE},
        false
    };
}();

inline auto TEXTURE_2D_DEPTH = []() -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_NEAREST, GL_NEAREST}, // filter
        {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT},
        false
    };
}();

inline auto TEXTURE_2D_HDR = []() -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_NEAREST, GL_NEAREST}, // filter
        {GL_RGBA16F, GL_RGBA, GL_FLOAT},
        false
    };
}();

inline auto TEXTURE_2D_BRDF = []() -> TextureAttributes
{
    return
    {
        GL_TEXTURE_2D,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_RG16F, GL_RG, GL_FLOAT},
        false
    };
}();

inline auto TEXTURE_CUBE_RGBA = []() -> TextureAttributes {
    return
    {
        GL_TEXTURE_CUBE_MAP,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_RGBA16F, GL_RGBA, GL_FLOAT},
        false
    };
}();

inline auto TEXTURE_CUBE_DEPTH = []() -> TextureAttributes {
    return
    {
        GL_TEXTURE_CUBE_MAP,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_NEAREST, GL_NEAREST}, // filter
        {GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT},
        false
    };
}();

inline auto TEXTURE_CUBE_RGB_FLOAT = []() -> TextureAttributes {
    return
    {
        GL_TEXTURE_CUBE_MAP,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_RGB16F, GL_RGB, GL_FLOAT},
        false
    };
}();

inline auto TEXTURE_CUBE_PREFILTER = []() -> TextureAttributes {
    return
    {
        GL_TEXTURE_CUBE_MAP,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_RGB16F, GL_RGB, GL_FLOAT},
        true
    };
}();

inline auto TEXTURE_CUBE_RGB = []() -> TextureAttributes {
    return
    {
        GL_TEXTURE_CUBE_MAP,
        {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, // warp
        {GL_LINEAR, GL_LINEAR}, // filter
        {GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE},
        false
    };
}();