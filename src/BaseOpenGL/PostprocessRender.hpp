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
    ShaderProgram _sp;
    PostprocessRender(std::string_view frag_path)
    {
        _sp.load_vs_file(SHADERS_PATH + "post_process/quad.vert");
        _sp.load_fs_file(frag_path);
        _sp.link();
    }
    void set(GLuint width, GLuint height) 
    {
        _width = width;
        _height = height;
    }
    void execute(GLuint input = 0)
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
    void render(GLuint input)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, _width, _height);
        _sp.use();
        _sp.active_sampler(0, input);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        VertexArray::render_empty_va();
    }
    operator GLuint()
    {
        return _result;
    }

};