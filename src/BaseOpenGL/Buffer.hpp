#pragma once

#include <glad/glad.h>
#include <vector>

#include "utility.hpp"

#define BUFFER Buffer::getInstance()
class Buffer
{
    SINGLETON(Buffer)
    std::vector<GLuint> _ids;
    void free()
    {
        glDeleteBuffers(_ids.size(), _ids.data());
    }
public:
    inline GLuint generate_vertex_buffer(GLsizeiptr size, void* data)
    {
        return generate_buffer(GL_ARRAY_BUFFER, size, data);
    }
    inline GLuint generate_element_buffer(GLsizeiptr size, void* data)
    {
        return generate_buffer(GL_ELEMENT_ARRAY_BUFFER, size, data);
    }
    GLuint generate_buffer(GLuint target, GLsizeiptr size, void* data, GLuint usage = GL_STATIC_DRAW)
    {
        glBindVertexArray(0);
        GLuint id;
        glGenBuffers(1, &id);
        glBindBuffer(target, id);
        glBufferData(target, size, data, usage);
        _ids.emplace_back(id);
        return _ids.back();
    }
};