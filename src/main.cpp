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



class PBR_render : public GLWidget
{

    Material rusted_iron{TEXTURE_PATH + "pbr/grass"};

    // 天空盒渲染pass
    GLuint _input_hdr = TEXTURE_MANAGER.load_hdr_texture(TEXTURE_PATH + "hdr/newport_loft.hdr");
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

    virtual void application() override
    {
        CAMERA.set_position({0.0, 0.0, 2.0});
        // budf_lut.execute();
        // equirect_pass.execute(_input_hdr);
        // convolution_pass.execute(equirect_pass);
        // prefilter_pass.execute(equirect_pass);

        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);        
        // gbuffer set
        gbuffer_fb.bind();
        gbuffer_fb.create_render_object(scrWidth, scrHeight);
        // depth_texture = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_DEPTH);
        // gbuffer_fb.attach_depth_texture(depth_texture);       
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
        
        light_sp.set_uniform("d_light.color", glm::vec3(1.0, 1.0, 1.0));
        light_sp.set_uniform("d_light.direction", glm::vec3(1.0, 1.0, 1.0));

    }

    virtual void render_loop() override
    {
        // gbuffer
        gbuffer_fb.bind();
        gbuffer_sp.use();
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
        glm::mat4 model(1.0);
        glm::mat4 view_model = CAMERA.get_view_matrix() * model;
        glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(view_model)));
        glm::mat4 proj_view_model = get_projection() * view_model;
        gbuffer_sp.set_uniform("view_model", view_model);
        gbuffer_sp.set_uniform("normal_matrix", normal_matrix);
        gbuffer_sp.set_uniform("proj_view_model", proj_view_model);
        gbuffer_sp.set_uniform("prev_proj_view_model", prev_proj_view_model);
        prev_proj_view_model = proj_view_model;
        rusted_iron.active(0);
        Shape::render_cube();//some render draw call
        gbuffer_fb.unbind();
        // light
        update_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);       
        glDepthMask(GL_FALSE);          
        light_sp.use();
        auto view_matrix = CAMERA.get_view_matrix();
        light_sp.set_uniform("inverse_view", glm::inverse(view_matrix));
        light_sp.set_uniform("inverse_projection", glm::inverse(get_projection()));
        light_sp.set_uniform("view_pos", CAMERA.get_position());
        light_sp.set_uniform("view_matrix3", glm::mat3(view_matrix));
        light_sp.set_uniform("view_matrix4", view_matrix);
        light_sp.set_uniform("inv_view_matrix3", glm::inverse(glm::mat3(view_matrix)));
        light_sp.active_sampler(0, gbtx_position);
        light_sp.active_sampler(1, gbtx_albdeo);
        light_sp.active_sampler(2, gbtx_normal);
        light_sp.active_sampler(3, gbtx_effects);
        light_sp.active_sampler(4, convolution_pass);
        light_sp.active_sampler(5, prefilter_pass);
        light_sp.active_sampler(6, budf_lut);
        light_sp.active_sampler(7, equirect_pass);
        VertexArray::render_empty_va();

        // // ready for forward render
        // int scrWidth, scrHeight;
        // glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        // glViewport(0, 0, scrWidth, scrHeight);          
        // glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer_fb);
        // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   
        // glBlitFramebuffer(0, 0, scrWidth, scrHeight, 0, 0, scrWidth, scrHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);        

        // _debug.render_texture(depth_texture);
        // _debug.render_texture(gbtx_position);

        // _skybox.render_texture(equirect_pass, get_projection());
        // _skybox.render_texture(prefilter_pass, get_projection());
        // _skybox.render_texture(prefilter, get_projection());
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