#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string_view>

#include "Shader.hpp"
#include "utility.hpp"

using ShaderType = GLenum;
using ShaderID = GLuint;

/**
 * @brief 着色器程序 用于管理管线相关资源 绑定着色器
 * 
 */
class ShaderProgram
{
public:

    ShaderProgram(ShaderProgram&& other) noexcept
    {
        _id = other._id;
        other._id = 0;
    }

    ShaderProgram() { _id = glCreateProgram(); }

    ShaderProgram(std::string_view vs, std::string_view fs)
    {
        _id = glCreateProgram();
        load_shader_file(GL_VERTEX_SHADER, vs);
        load_shader_file(GL_FRAGMENT_SHADER, fs);
        link();
    }

    ShaderProgram(std::string_view vs, std::string_view gs,std::string_view fs)
    {
        _id = glCreateProgram();
        load_shader_file(GL_VERTEX_SHADER, vs);
        load_shader_file(GL_GEOMETRY_SHADER, gs);
        load_shader_file(GL_FRAGMENT_SHADER, fs);
        link();
    }

    ~ShaderProgram()
    {
        if (_id) 
        {
            glDeleteProgram(_id);
            _id = 0;
        }

    }

    void load_shader_file(ShaderType type, std::string_view path)
    {
        std::string src;
        utility::read_file(path, src);
        load_shader_src(type, src);
    }
    void load_shader_src(ShaderType type, std::string_view src)
    {
        Shader s{ type };
        s.init_from_src(src);
        glAttachShader(_id, s.id());
    }

    inline void load_vs_file(std::string_view path) { load_shader_file(GL_VERTEX_SHADER, path); }
    inline void load_vs_src(std::string_view src) { load_shader_src(GL_VERTEX_SHADER, src); }
    inline void load_fs_file(std::string_view path) { load_shader_file(GL_FRAGMENT_SHADER, path); }
    inline void load_fs_src(std::string_view src) { load_shader_src(GL_FRAGMENT_SHADER, src); }
    inline void load_gs_file(std::string_view path) { load_shader_file(GL_GEOMETRY_SHADER, path); }
    inline void load_gs_src(std::string_view src) { load_shader_src(GL_GEOMETRY_SHADER, src); }
    inline GLuint get_id() const { return _id; };
    inline void use() const { glUseProgram(_id); }

    void link()
    {
        glLinkProgram(_id);
        utility::checkCompileErrors(_id, "PROGRAME");
    }

    void active_sampler(int offset, GLuint texture, GLenum target = GL_TEXTURE_2D) const
    {
        glActiveTexture(GL_TEXTURE0 + offset);
        glBindTexture(target, texture);
    }

    template<typename T>
    void set_uniform(std::string_view name, const T& value) const;

    template<>
    void set_uniform<int>(std::string_view name, const int& value) const
    {
        glUniform1i(glGetUniformLocation(_id, name.data()), value);
    }

    template<>
    void set_uniform<float>(std::string_view name, const float& value) const
    {
        glUniform1f(glGetUniformLocation(_id, name.data()), value);
    }

    template<>
    void set_uniform<bool>(std::string_view name, const bool& value) const
    {
        glUniform1i(glGetUniformLocation(_id, name.data()), value ? 1 : 0);
    }

    template<>
    void set_uniform<GLuint>(std::string_view name, const GLuint& value) const
    {
        glUniform1ui(glGetUniformLocation(_id, name.data()), value);
    }

    template<>
    void set_uniform<glm::vec2>(std::string_view name, const glm::vec2& value) const
    {
        glUniform2f(glGetUniformLocation(_id, name.data()), value.x, value.y);
    }

    template<>
    void set_uniform<glm::vec3>(std::string_view name, const glm::vec3& value) const
    {
        glUniform3f(glGetUniformLocation(_id, name.data()), value.x, value.y, value.z);
    }

    template<>
    void set_uniform<glm::vec4>(std::string_view name, const glm::vec4& value) const
    {
        glUniform4f(glGetUniformLocation(_id, name.data()), value.x, value.y, value.z, value.w);
    }

    template<>
    void set_uniform<glm::mat2>(std::string_view name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(_id, name.data()), 1, GL_FALSE, &mat[0][0]);
    }

    template<>
    void set_uniform<glm::mat3>(std::string_view name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(_id, name.data()), 1, GL_FALSE, &mat[0][0]);
    }

    template<>
    void set_uniform<glm::mat4>(std::string_view name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(_id, name.data()), 1, GL_FALSE, &mat[0][0]);
    }

    void set_sampler(int sampler_id, std::string_view sampler_name)
    {
        set_uniform<int>(sampler_name, sampler_id);
    }

    template<typename T>
    void set_uniform_vector(std::string_view name, GLsizei rank, void* data) const
    {
        if constexpr (std::is_same_v<T, float>) 
        {
            glUniform1fv(glGetUniformLocation(_id, name.data()), 4, (GLfloat*)data);
        }
        else if constexpr (std::is_same_v<T, glm::vec2>) 
        {
            glUniform2fv(glGetUniformLocation(_id, name.data()), 4, (GLfloat*)data);
        }
        else if constexpr (std::is_same_v<T, glm::vec3>) 
        {
            glUniform3fv(glGetUniformLocation(_id, name.data()), 4, (GLfloat*)data);
        }
        else 
        {
            //error;
        }
    }

private:
    GLuint _id;
};


