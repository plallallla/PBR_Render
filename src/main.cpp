#include "Camera.hpp"
#include "FrameBuffer.hpp"
#include "GLWidget.hpp"
#include "QuadRender.hpp"
#include "Precompution.hpp"
#include "Shape.hpp"
#include "SkyboxRender.hpp"
#include "Material.hpp"
#include "Model.hpp"

#include "PostRender.hpp"
#include "Texture.hpp"
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


    FrameBuffer gbuffer_fb;
    GLuint gbtx_position;
    GLuint gbtx_albdeo;
    GLuint gbtx_normal;
    GLuint gbtx_effect;

    glm::mat4 prev_proj_view_model;

    ShaderProgram gbuffer_sp
    {
        SHADERS_PATH + "render/gbuffer.vert",
        SHADERS_PATH + "render/gbuffer.frag" 
    };  

    PostRender _debug;

    virtual void application() override
    {
        budf_lut.execute();
        equirect_pass.execute(_input_hdr);
        convolution_pass.execute(equirect_pass);
        prefilter_pass.execute(equirect_pass);

        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);        

        gbuffer_fb.bind();
        gbuffer_fb.create_render_object(scrWidth, scrHeight);
        gbtx_position = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA16F);
        gbtx_albdeo = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA);
        gbtx_normal = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA16F);
        gbtx_effect = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGB16F);
        gbuffer_fb.attach_color_texture(0, gbtx_position);
        gbuffer_fb.attach_color_texture(1, gbtx_albdeo);
        gbuffer_fb.attach_color_texture(2, gbtx_normal);
        gbuffer_fb.attach_color_texture(3, gbtx_effect);
        gbuffer_fb.active_draw_buffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
        gbuffer_fb.unbind();


    }

    virtual void render_loop() override
    {
        gbuffer_fb.bind();
        gbuffer_sp.use();
        rusted_iron.active(0);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        
        glm::mat4 model(1.0);
        glm::mat4 view_model = CAMERA.get_view_matrix() * model;
        glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(view_model)));
        glm::mat4 proj_view_model = get_projection() * CAMERA.get_view_matrix() * model;
        gbuffer_sp.set_uniform("view_model", view_model);
        gbuffer_sp.set_uniform("normal_matrix", normal_matrix);
        gbuffer_sp.set_uniform("proj_view_model", proj_view_model);
        gbuffer_sp.set_uniform("prev_proj_view_model", prev_proj_view_model);
        prev_proj_view_model = proj_view_model;
        Shape::render_sphere();//some render draw call
        gbuffer_fb.unbind();
        
        update_viewport();
        _debug.render_texture(gbtx_position);

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