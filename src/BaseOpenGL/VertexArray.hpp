#pragma once
#include <OpenGL/gltypes.h>
#include <cstddef>
#include <glad/glad.h>
#include <vector>
#include <map>

struct VertexAttribute
{
    GLuint _type{ GL_FLOAT };
    GLuint _count{ 0 };
    bool _normalized{ false };
    bool _instanced{ false };
    GLuint _divisor{ 0 };
    VertexAttribute(GLuint type, GLuint count, bool normalized, bool instanced, GLuint divisor)
    : _type(type)
    , _count(count)
    , _normalized(normalized)
    , _instanced(instanced)
    , _divisor(divisor) 
    {}
    static GLuint get_type_length(GLuint type)
    {
        static std::map<GLuint, GLuint> type_length
        {
            { GL_FLOAT, 4 }, { GL_UNSIGNED_INT, 4}, { GL_UNSIGNED_BYTE, 1},
        };
        return type_length[type];
    }
};
using VertexAttributes = std::vector<VertexAttribute>;

struct BufferLayout
{
    VertexAttributes _attributes;
    GLuint _stride{ 0 };
    void add_attribute(GLuint type, GLuint count, bool normalized = true, bool instanced = false, GLuint divisor = 0)
    {

        _attributes.emplace_back(type, count, normalized, instanced, divisor);
        _stride += count * VertexAttribute::get_type_length(type);
    }
};

static inline auto P_LAYOUT = []() -> BufferLayout
{
    BufferLayout layout;
    layout.add_attribute(GL_FLOAT, 3); // position
    return layout;
}();

static inline auto PN_LAYOUT = []() -> BufferLayout
{
    BufferLayout layout;
    layout.add_attribute(GL_FLOAT, 3); // position
    layout.add_attribute(GL_FLOAT, 3); // normal
    return layout;
}();

static inline auto PT_LAYOUT = []() -> BufferLayout
{
    BufferLayout layout;
    layout.add_attribute(GL_FLOAT, 3); // position
    layout.add_attribute(GL_FLOAT, 2); // texture uv
    return layout;
}();

static inline auto PNT_LAYOUT = []() -> BufferLayout
{
    BufferLayout layout;
    layout.add_attribute(GL_FLOAT, 3); // position
    layout.add_attribute(GL_FLOAT, 3); // normal
    layout.add_attribute(GL_FLOAT, 2); // texture uv
    return layout;
}();

static inline auto PNTTB_LAYOUT = []() -> BufferLayout
{
    BufferLayout layout;
    layout.add_attribute(GL_FLOAT, 3); // position
    layout.add_attribute(GL_FLOAT, 3); // normal
    layout.add_attribute(GL_FLOAT, 2); // texture uv
    layout.add_attribute(GL_FLOAT, 3); // tangent
    layout.add_attribute(GL_FLOAT, 3); // bitangent
    return layout;
}();

class VertexArray
{
    friend class Mesh;
    GLuint _id{ 0 };
    GLuint _attributes_ct{ 0 };
    void attach_layout(const BufferLayout& layout)
    {
        size_t offset = 0;
        for (size_t i = 0; i < layout._attributes.size(); i++)
        {
            const auto& attribute = layout._attributes[i];
            glVertexAttribPointer
            (
                _attributes_ct, 
                attribute._count, 
                attribute._type, 
                attribute._normalized ? GL_TRUE : GL_FALSE, 
                layout._stride, 
                (void*)offset
            );
            offset += (size_t)attribute._count * VertexAttribute::get_type_length(attribute._type);
            if (attribute._instanced) 
            {
                glVertexAttribDivisor(_attributes_ct, attribute._divisor);
            }
            glEnableVertexAttribArray(_attributes_ct++);
        }
    }

public:
    VertexArray() { glGenVertexArrays(1, &_id); }
    ~VertexArray() { if (_id) glDeleteVertexArrays(1, &_id); }
    inline void bind() const { glBindVertexArray(_id); }
    inline void unbind() const { glBindVertexArray(0); }
    void attach_buffer(const BufferLayout& layout, GLuint b_buffer_id, GLuint e_buffer_id)
    {
        glBindVertexArray(_id);
        glBindBuffer(GL_ARRAY_BUFFER, b_buffer_id);
        attach_layout(layout);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_buffer_id);
        glBindVertexArray(0);
    }
    void attach_element_buffer(GLuint buffer_id)
    {
        glBindVertexArray(_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);
        glBindVertexArray(0);
    }
    void attach_vertex_buffer(const BufferLayout& layout, GLuint buffer_id)
    {
        glBindVertexArray(_id);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
        attach_layout(layout);
        glBindVertexArray(0);
    }
};