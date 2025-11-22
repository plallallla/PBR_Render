#include "Camera.hpp"
#include "FrameBuffer.hpp"
#include "GLWidget.hpp"
#include "QuadRender.hpp"
#include "Precompution.hpp"
#include "ShaderProgram.hpp"
#include "Shape.hpp"
#include "SkyboxRender.hpp"
#include "Material.hpp"
#include "Model.hpp"

#include "PostRender.hpp"
#include "Texture.hpp"
#include "TextureAttributes.hpp"
#include "VertexArray.hpp"
#include "utility.hpp"
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>


    std::vector<glm::vec3> lightPositions
    {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
    };
    std::vector<glm::vec3> lightColors
    {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };



class PBR_render : public GLWidget
{

    Material rusted_iron{TEXTURE_PATH + "pbr/rusted_iron"};
    Material gold{TEXTURE_PATH + "pbr/gold"};

    // 天空盒渲染pass
    GLuint _input_hdr = TEXTURE_MANAGER.auto_load_texture(TEXTURE_PATH + "hdr/newport_loft.hdr");
    SkyboxRender _skybox;

    // 预处理渲染
    BRDF_LUT budf_lut;
    EquirectConvertRender equirect_pass;
    ConvolutionIBLRender convolution_pass;
    PrefilterIBLRender prefilter_pass;

    // 着色器
    ShaderProgram light_sp
    {
        SHADERS_PATH + "render/light.vert",
        SHADERS_PATH + "render/light.frag" 
    };

    // gbuffer资源
    FrameBuffer gbuffer_fb;
    GLuint depth_texture;
    GLuint gbtx_position;
    GLuint gbtx_albdeo;
    GLuint gbtx_normal;
    GLuint gbtx_effects;
    glm::mat4 prev_proj_view_model;
    ShaderProgram gbuffer_sp
    {
        SHADERS_PATH + "render/gbuffer.vert",
        SHADERS_PATH + "render/gbuffer.frag" 
    };    

    PostRender _debug;

    QuadRender _quad;

    ShaderProgram _debug_gbuffer_sp
    {
        SHADERS_PATH + "deffered/gbuffer.vert",
        SHADERS_PATH + "deffered/gbuffer.frag" 
    };    
    
    ShaderProgram _debug_light_sp
    {
        SHADERS_PATH + "common_vertex/quad.vert",
        SHADERS_PATH + "deffered/light.frag" 
    };       


    virtual void application() override
    {

        stbi_set_flip_vertically_on_load(true);        

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        CAMERA.set_position({0.0, 0.0, 4.0});
        budf_lut.execute();
        equirect_pass.execute(_input_hdr);
        convolution_pass.execute(equirect_pass);
        prefilter_pass.execute(equirect_pass);
        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);        
        // gbuffer set
        gbuffer_fb.bind();
        gbuffer_fb.create_render_object(scrWidth, scrHeight);
        gbtx_position = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA16F);
        gbuffer_fb.attach_color_texture(0, gbtx_position);
        gbtx_albdeo = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA);
        gbuffer_fb.attach_color_texture(1, gbtx_albdeo);
        gbtx_normal = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA16F);
        gbuffer_fb.attach_color_texture(2, gbtx_normal);
        gbtx_effects = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGB16F);
        gbuffer_fb.attach_color_texture(3, gbtx_effects);
        gbuffer_fb.active_draw_buffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
        gbuffer_fb.checkFramebufferStatus();
        gbuffer_fb.unbind();

        _debug_light_sp.use();
        _debug_light_sp.set_sampler(0, "s_position");
        _debug_light_sp.set_sampler(1, "s_normal");
        _debug_light_sp.set_uniform("d_light.color", glm::vec3(10.0, 10.0, 10.0));
        _debug_light_sp.set_uniform("d_light.direction", glm::vec3(1.0, 1.0, 1.0));
        _debug_light_sp.set_uniform("projection", get_projection());          


        // gbuffer
        Material::set_samplers(gbuffer_sp, 0);
             
        // light
        light_sp.use();
        light_sp.set_sampler(0, "s_position");
        light_sp.set_sampler(1, "s_albedo");
        light_sp.set_sampler(2, "s_normal");
        light_sp.set_sampler(3, "s_effects");
        light_sp.set_sampler(4, "ibl_convolution");
        light_sp.set_sampler(5, "ibl_prefilter");
        light_sp.set_sampler(6, "ibl_brdf_lut");
        light_sp.set_sampler(7, "env_cube");     
        light_sp.set_uniform("d_light.color", glm::vec3(10.0, 10.0, 10.0));
        light_sp.set_uniform("d_light.direction", glm::vec3(1.0, 1.0, 1.0));
    }



    void deffered_render()
    {
        // gbuffer
        gbuffer_fb.bind();
        gbuffer_sp.use();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        static const float clear_g_position[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, clear_g_position);// 写入默认深度值为1        
        glm::mat4 projection = get_projection();
        glm::mat4 view = CAMERA.get_view_matrix();
        glm::mat4 model(1.0);
        glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(model)));
        glm::mat4 proj_view_model = projection * view * model;
        gbuffer_sp.set_uniform("model", model);
        gbuffer_sp.set_uniform("eye_position", CAMERA.get_position());
        gbuffer_sp.set_uniform("normal_matrix", normal_matrix);
        gbuffer_sp.set_uniform("proj_view_model", proj_view_model);
        gbuffer_sp.set_uniform("prev_proj_view_model", prev_proj_view_model);
        prev_proj_view_model = proj_view_model;
        rusted_iron.active(0); 
        // gold.active(0); 
        Shape::render_sphere();//some render draw call
        gbuffer_fb.unbind();
        // light
        update_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);   
        glDepthMask(GL_FALSE);
        light_sp.use();
        light_sp.set_uniform("eye_position", CAMERA.get_position());
        light_sp.set_uniform("cube_uv_trans", glm::inverse(glm::mat4(glm::mat3(view))) * glm::inverse(projection));        
        light_sp.active_sampler(0, gbtx_position);
        light_sp.active_sampler(1, gbtx_albdeo);
        light_sp.active_sampler(2, gbtx_normal);
        light_sp.active_sampler(3, gbtx_effects);
        light_sp.active_sampler(4, convolution_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(5, prefilter_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(6, budf_lut);
        light_sp.active_sampler(7, equirect_pass, GL_TEXTURE_CUBE_MAP);
        VertexArray::render_empty_va();        
    }

    virtual void render_loop() override
    {
        deffered_render();
        // 拷贝gbuffer的深度缓存用于深度测试
        // int scrWidth, scrHeight;
        // glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        // glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer_fb);
        // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        // glBlitFramebuffer(0, 0, scrWidth, scrHeight, 0, 0, scrWidth, scrHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        // _skybox.render_texture(equirect_pass, get_projection());
    }

public:
    PBR_render(int width, int height, std::string_view title) : GLWidget(width,height,title) 
    {
    }
};


int main()
{
    PBR_render pbr_render_widget{900, 800, "pbr_render"};
    pbr_render_widget.render();
    return 0;
}