#include "ShaderProgram.hpp"
#include "VertexArray.hpp"
#include "Buffer.hpp"
#include "Camera.hpp"

class SkyboxRender
{
    static std::string default_vs_src;
    static std::string default_fs_src;
    void set_up_va()
    {
        float skyboxVertices[] = 
        {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
        };
        _va.attach_vertex_buffer(P_LAYOUT, BUFFER.generate_buffer(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices));
    }
public:
    ShaderProgram _sp;
    VertexArray _va;
    SkyboxRender()
    {
        _sp.load_vs_src(default_vs_src);
        _sp.load_fs_src(default_fs_src);
        _sp.link();
        set_up_va();
    }
    void render_texture(GLuint texture, glm::mat4 projection)
    {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        _sp.use();
        _sp.set_uniform("view", glm::mat4(glm::mat3(CAMERA.get_view_matrix())));//移除位移
        _sp.set_uniform("projection", projection);
        _sp.active_sampler(0, texture, GL_TEXTURE_CUBE_MAP);
        _va.bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);    
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);        
    }
};

inline std::string SkyboxRender::default_vs_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;
uniform mat4 projection;
uniform mat4 view;
void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  
)";

inline std::string SkyboxRender::default_fs_src = R"(
#version 330 core
out vec4 FragColor;
in vec3 TexCoords;
uniform samplerCube skybox;
void main()
{    
    FragColor = texture(skybox, TexCoords);
}
)";