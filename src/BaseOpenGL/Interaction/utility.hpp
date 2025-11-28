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

#define HAS_RESULT protected: GLuint _result; public: operator GLuint() { return _result; }

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
        auto& check = type != "PROGRAME" ? glGetShaderiv : glGetProgramiv;
        auto status = type != "PROGRAME" ? GL_COMPILE_STATUS : GL_LINK_STATUS;
        check(id, status, &success);
        return success;
    }

}