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
    HAS_RESULT;
    ShaderProgram _sp;
    PostprocessRender(std::string_view frag_path)
    {
        _sp.load_vs_file(SHADERS_PATH + "post_process/quad.vert");
        _sp.load_fs_file(frag_path);
        _sp.link();
    }
    void set(GLuint width, GLuint height, const TextureAttributes& attributes = TEXTURE_2D_RGBA)
    {
        _width = width;
        _height = height;
        _result = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, attributes);       
    }
    void execute(GLuint input = 0)
    {
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
    void execute(std::vector<GLuint> input)
    {
        _fb.bind();
        _fb.attach_color_texture(0, _result);
        glViewport(0, 0, _width, _height);
        _sp.use();
        for (int i = 0; i < input.size(); i++)
        {
            _sp.active_sampler(i, input[i]);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        VertexArray::render_empty_va();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);        
    }
    void render(std::vector<GLuint> input)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, _width, _height);
        _sp.use();
        for (int i = 0; i < input.size(); i++)
        {
            _sp.active_sampler(i, input[i]);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        VertexArray::render_empty_va();
    }    

};