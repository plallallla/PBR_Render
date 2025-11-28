#include "Camera.hpp"
#include "GLWidget.hpp"
#include "PrecomputedRender.hpp"
#include "PostprocessRender.hpp"
#include "ShaderProgram.hpp"
#include "Shadow.hpp"
#include "SkyboxRender.hpp"
#include "Material.hpp"
#include "VertexArray.hpp"
#include "utility.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>

#include "Model.hpp"

class PBR_render : public GLWidget
{

    Material rusted_iron{TEXTURE_PATH + "pbr/rusted_iron"};
    Material woodfloor{TEXTURE_PATH + "pbr/woodfloor"};

    // 天空盒渲染pass
    GLuint _input_hdr = TEXTURE_MANAGER.auto_load_texture(TEXTURE_PATH + "hdr/newport_loft.hdr");
    SkyboxRender _skybox;

    // 预处理渲染
    BRDF_LUT budf_lut;
    EquirectConvertRender equirect_pass;
    ConvolutionIBLRender convolution_pass;
    PrefilterIBLRender prefilter_pass;

    // light资源
    FrameBuffer light_fb;
    GLuint light_result_texture;
    ShaderProgram light_sp
    {
        SHADERS_PATH + "render/light.vert",
        SHADERS_PATH + "render/light.frag" 
    };

    Light direction_light
    {
        light_type::directional,
        glm::vec3(10.0, 10.0, 10.0),
        DirectionalLight{glm::vec3(1.0, 1.0, 1.0)}
    };    
    Shadow direction_shadow{direction_light, 1024, 1024};    

    // gbuffer资源
    FrameBuffer gbuffer_fb;
    GLuint depth_texture;
    GLuint gbtx_position;
    GLuint gbtx_albdeo;
    GLuint gbtx_normal;
    GLuint gbtx_effects;
    ShaderProgram gbuffer_sp
    {
        SHADERS_PATH + "render/gbuffer.vert",
        SHADERS_PATH + "render/gbuffer.frag" 
    };    

    // 后处理
    PostprocessRender _display_pass{ SHADERS_PATH + "post_process/display.frag" };
    PostprocessRender _color_correction_pass{ SHADERS_PATH + "post_process/color_correction.frag" };
    PostprocessRender _fxaa_pass{ SHADERS_PATH + "post_process/fxaa.frag" };
    PostprocessRender _motion_blur_pass{ SHADERS_PATH + "post_process/motion_blur.frag" };

    // glm::mat4 prev_proj_view_model;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 prev_projection;
    glm::mat4 prev_view;

    Model teapot_obj;
    glm::mat4 teapot_model;
    Model floor_obj;
    glm::mat4 floor_model;

    virtual void application() override
    {

        stbi_set_flip_vertically_on_load(true);
        
        teapot_obj.load_single_obj({"../resources/obj/teapot.obj"});
        teapot_model = glm::mat4(1.0);
        teapot_model = glm::translate(teapot_model, {0.0, 2.0, 0.0});
        floor_obj.load_single_obj({"../resources/obj/floor.obj"});
        floor_model = glm::mat4(1.0);
        floor_model = glm::scale(floor_model, {10.0, 10.0, 10.0});

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        CAMERA.set_position({0.0, 3.0, 10.0});
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

        // gbuffer
        Material::set_samplers(gbuffer_sp, 0);
             
        // light
        light_fb.bind();
        light_fb.create_render_object(scrWidth, scrHeight);
        light_result_texture = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA16F);
        light_fb.attach_color_texture(0, light_result_texture);
        light_fb.active_draw_buffers({GL_COLOR_ATTACHMENT0});
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

        // postprocess
        _display_pass.set(scrWidth, scrHeight);
        _color_correction_pass.set(scrWidth, scrHeight);

        _fxaa_pass.set(scrWidth, scrHeight);
        _fxaa_pass._sp.use();
        _fxaa_pass._sp.set_uniform("frag_size", glm::vec2(1.0 / scrWidth, 1.0 / scrHeight));

        _motion_blur_pass.set(scrWidth, scrHeight);
        _motion_blur_pass._sp.use();
        _motion_blur_pass._sp.set_sampler(0, "screenTexture");
        _motion_blur_pass._sp.set_sampler(1, "gEffects");

        direction_shadow.begin();

        direction_shadow._sp->set_uniform("model", teapot_model);
        teapot_obj.render_elements();
        direction_shadow._sp->set_uniform("model", floor_model);
        floor_obj.render_elements();
        
        direction_shadow.end();

    }

    void render_object(Model& m, const glm::mat4 model, const Material& material)
    {
        gbuffer_sp.set_uniform("model", model);
        gbuffer_sp.set_uniform("eye_position", CAMERA.get_position());
        glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(model)));
        gbuffer_sp.set_uniform("normal_matrix", normal_matrix);
        gbuffer_sp.set_uniform("proj_view_model", projection * view * model);
        gbuffer_sp.set_uniform("prev_proj_view_model", prev_projection * prev_view * model);
        material.active(0);
        m.render_elements();        
    }

    void model_render(const Model& m, const glm::mat4& model, Material& material)
    {
        glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(model)));
        gbuffer_sp.set_uniform("model", model);        
        gbuffer_sp.set_uniform("normal_matrix", normal_matrix);
        gbuffer_sp.set_uniform("proj_view_model", projection * view * model);
        gbuffer_sp.set_uniform("prev_proj_view_model", prev_projection * prev_view * model);
    }
    void geometry_render()
    {
        projection = get_projection();
        view = CAMERA.get_view_matrix(); 
        gbuffer_fb.bind();
        gbuffer_sp.use();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        static const float clear_g_position[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, clear_g_position);// 写入默认深度值为1        
        
        render_object(teapot_obj, teapot_model, rusted_iron);
        render_object(floor_obj, floor_model, woodfloor);

        gbuffer_fb.unbind();
        prev_projection = projection;
        prev_view = view;
    }    

    void light_render()
    {
        // light
        light_fb.bind();
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
        light_fb.unbind();
    }

    void postprocess()
    {
        _motion_blur_pass.execute({light_result_texture, gbtx_effects});
        _color_correction_pass.execute(_motion_blur_pass);
        _fxaa_pass.execute(_color_correction_pass);
        _display_pass.render(_fxaa_pass);
    }

    virtual void render_loop() override
    {
        // deffered_render();
        // geometry_render();
        // light_render();
        // postprocess();
        _display_pass.render(direction_shadow);
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