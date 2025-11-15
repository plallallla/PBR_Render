#pragma once
#include <glad/glad.h>
#include <stdexcept>
#include <vector>

class FrameBuffer 
{
public:
    FrameBuffer() { glGenFramebuffers(1, &_id); }
    ~FrameBuffer() { if (_id) glDeleteFramebuffers(1, &_id); if (_rbo) glDeleteRenderbuffers(1, &_rbo); }
    inline void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, _id); }
    inline void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void set_draw_read(GLenum draw, GLenum read)
    {
        glDrawBuffer(draw);
        glReadBuffer(read);        
    }

    operator GLuint() const
    {
        return _id;
    }

    // 传入attachment数组以开启MRT, 如{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2}
    void active_draw_buffers(const std::vector<GLenum>& buffers) const
    {
        glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
    }

    void attach_color_texture(GLuint offset, GLuint texture, GLenum target = GL_TEXTURE_2D)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + offset, target, texture, 0);
    }

    void attach_depth_texture(GLuint texture)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
    }

    void attach_depth_texture_array(GLuint texture)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
    }

    void attach_stencil_texture(GLuint texture)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
    }

    void create_render_object(int width, int height)
    {
        if (!_rbo) glGenRenderbuffers(1, &_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);
    }

    bool check_status() const
    {
        bind();
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        unbind();
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void checkFramebufferStatus() const
    {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch (status)
        {
            case GL_FRAMEBUFFER_COMPLETE:
            {
                return;
            }
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            {
                throw std::runtime_error("Framebuffer error: INCOMPLETE_ATTACHMENT");
                return;
            }
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            {
                throw std::runtime_error("Framebuffer error: MISSING_ATTACHMENT");
                return;
            }
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            {
                throw std::runtime_error("Framebuffer error: INCOMPLETE_DRAW_BUFFER");
                return;
            }
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            {
                throw std::runtime_error("Framebuffer error: INCOMPLETE_READ_BUFFER");
                return;
            }
            case GL_FRAMEBUFFER_UNSUPPORTED:
            {
                throw std::runtime_error("Framebuffer error: UNSUPPORTED");
                return;
            }
            default:
            {
                throw std::runtime_error("Framebuffer error: UNKNOWN");
                return;
            }
        }
    }

private:
    GLuint _id{ 0 };
    GLuint _rbo{ 0 };
};


