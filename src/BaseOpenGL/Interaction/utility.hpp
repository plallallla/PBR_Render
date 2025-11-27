#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
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
            std::cout << "ERROR::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
    }

    inline bool checkCompileErrors(GLuint id, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        auto& check = type != "PROGRAME" ? glGetShaderiv : glGetProgramiv;
        auto status = type != "PROGRAME" ? GL_COMPILE_STATUS : GL_LINK_STATUS;
        check(id, status, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "->\n" << infoLog << std::endl;
            LOG.info("\nSHADER_COMPILATION_ERROR of type: " + type + "\n" + infoLog);
            return false;
        }
        return true;
    }

}