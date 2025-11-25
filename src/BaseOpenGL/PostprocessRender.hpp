#include <vector>
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"
#include "FrameBuffer.hpp"
#include "Texture.hpp"

class PostprocessRender
{
protected:
    GLuint _result = 0;
    FrameBuffer _fb;
    GLuint _width;
    GLuint _height;
public:
    PostprocessRender(GLuint width = 512, GLuint height = 512) : _width{ width }, _height{ height } {}
    void set(GLuint width, GLuint height) 
    {
        _width = width;
        _height = height;
    }
    virtual void execute(GLuint input = 0) = 0;
    virtual void render(GLuint input) = 0;
    operator GLuint()
    {
        return _result;
    }

};

class DisplayRender : public PostprocessRender
{
    ShaderProgram _sp
    {
        SHADERS_PATH + "post_process/quad.vert", 
        SHADERS_PATH + "post_process/display.frag"
    };
public:
    DisplayRender(GLuint width = 512, GLuint height = 512) : PostprocessRender{ width, height } {}
    virtual void execute(GLuint input = 0) override
    {
        _result = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, TEXTURE_2D_RGBA);       
        _fb.bind();
        _fb.attach_color_texture(0, _result);
        glViewport(0, 0, _width, _height);
        _sp.use();
        _sp.active_sampler(0, input);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        VertexArray::render_empty_va();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);        
    }
    virtual void render(GLuint input) override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, _width, _height);
        _sp.use();
        _sp.active_sampler(0, input);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        VertexArray::render_empty_va();
    }
};
