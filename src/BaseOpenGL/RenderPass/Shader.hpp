#pragma once
#include "utility.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

/**
 * @brief 着色器类 封装了部分着色器的操作
 * 
 */
class Shader
{
public:
    explicit Shader(GLuint type) : _type(type) {};

    virtual ~Shader()
    {
        if (_id)
        {
            glDeleteShader(_id);
            _id = 0;
        }
    }

    virtual void init_from_src(std::string_view src)
    {
        const char* vShaderCode = src.data();
        _id = glCreateShader(_type);
        glShaderSource(_id, 1, &vShaderCode, NULL);
        glCompileShader(_id);
        utility::checkCompileErrors(_id, _name[_type]);
    }

    inline bool empty() const
    {
        return 0 == _id;
    };

    inline GLuint id() const
    {
        return _id;
    };

private:
    GLuint _type;
    GLuint _id = 0;
    std::unordered_map<GLuint, std::string> _name
    {
        {GL_VERTEX_SHADER,"GL_VERTEX_SHADER"},
        {GL_FRAGMENT_SHADER,"GL_FRAGMENT_SHADER"},
        {GL_GEOMETRY_SHADER,"GL_GEOMETRY_SHADER"},
    };
};
