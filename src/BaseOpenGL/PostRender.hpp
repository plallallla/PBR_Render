#include <vector>
#include "ShaderProgram.hpp"

class PostRender
{
    static std::string default_vs_src;
    static std::string default_fs_src;
public:
    ShaderProgram _sp;
    GLuint va;
    PostRender(std::string_view fs_path)
    {
        _sp.load_vs_src(default_vs_src);
        _sp.load_fs_file(fs_path);
        _sp.link();
    }
    PostRender()
    {
        _sp.load_vs_src(default_vs_src);
        _sp.load_fs_src(default_fs_src);
        _sp.link();
    }
    void render_texture(GLuint texture)
    {
        if (!va) glGenVertexArrays(1, &va);
        glBindVertexArray(va);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        _sp.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    void render_texture(const std::vector<GLuint>& textures)
    {
        if (!va) glGenVertexArrays(1, &va);
        glBindVertexArray(va);
        _sp.use();
        for (int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};

inline std::string PostRender::default_vs_src = R"(
#version 330 core
out vec2 uv;
void main()
{
    uv = vec2
    (
        (gl_VertexID & 1) << 2,
        (gl_VertexID & 2) << 1
    );
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}
)";

inline std::string PostRender::default_fs_src = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D screenTexture;
void main()
{
    FragColor = vec4(texture(screenTexture, uv).rgb, 1.0);
} 
)";