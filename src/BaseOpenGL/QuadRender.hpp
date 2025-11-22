#include <vector>
#include "ShaderProgram.hpp"
#include "Shape.hpp"

class QuadRender
{
    static std::string default_vs_src;
    static std::string default_fs_src;
public:
    ShaderProgram _sp;
    QuadRender(std::string_view fs_path)
    {
        _sp.load_vs_src(default_vs_src);
        _sp.load_fs_file(fs_path);
        _sp.link();
    }
    QuadRender()
    {
        _sp.load_vs_src(default_vs_src);
        _sp.load_fs_src(default_fs_src);
        _sp.link();
    }
    void render_texture(GLuint texture)
    {
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        _sp.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        Shape::render_quad();
    }
    void render_texture(const std::vector<GLuint>& textures)
    {
        _sp.use();
        for (int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
        }
        Shape::render_quad();
    }
};

inline std::string QuadRender::default_vs_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

inline std::string QuadRender::default_fs_src = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
void main()
{
    //FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
    FragColor = vec4(1.0, 1.0,1.0, 1.0);
} 
)";