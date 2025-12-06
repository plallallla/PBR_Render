#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

#define SINGLETON(ClassName) \
public: \
    static ClassName& getInstance() { \
        static ClassName instance; \
        return instance; \
    } \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete; \
private: \
    ClassName() = default; \
    \
    ~ClassName() = default; \

#include "LogHelper.h"

#define HAS_RESULT protected: GLuint _result{0}; public: operator GLuint() { return _result; }

namespace utility
{

    inline void read_file(std::string_view path, std::string& content)
    {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            file.open(path);
            std::stringstream ss;
            ss << file.rdbuf();
            content = ss.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::FILE_NOT_SUCCESSFULLY_READ[" << path << "]:" << e.what() << std::endl;
        }
    }

    inline bool compile_check(GLuint id, std::string& err)
    {
        GLint success;
        GLchar infoLog[1024];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            err = std::string{ infoLog };
        }
        return success;
    }

    inline bool link_check(GLuint id, std::string& err)
    {
        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            err = std::string{ infoLog };
        }
        return success;
    }

}