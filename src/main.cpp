#include "Camera.hpp"
#include "FrameBuffer.hpp"
#include "GLWidget.hpp"
#include "PrecomputedRender.hpp"
#include "PostprocessRender.hpp"
#include "ShaderProgram.hpp"
#include "Shadow.hpp"
#include "SkyboxRender.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "TextureAttributes.hpp"
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
    Shadow direction_shadow{direction_light, 2048, 2048};    

    Light point_light
    {
        light_type::point,
        glm::vec3(4.0, 4.0, 4.0),
        PointLight{glm::vec3(-1.0, 1.0, 1.0), {0.0, 1.0, 0.1}}
    };
    Shadow point_shadow{point_light, 2048, 2048};    

    // gbuffer资源
    FrameBuffer gbuffer_fb;
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
    PostprocessRender _depth24_debug{ SHADERS_PATH + "post_process/depth24_debug.frag" };

    // glm::mat4 prev_proj_view_model;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 prev_projection;
    glm::mat4 prev_view;

    Model teapot_obj;
    glm::mat4 teapot_model;
    Model floor_obj;
    glm::mat4 floor_model;

    // point shadow
    FrameBuffer frame;
    ShaderProgram point_depth_shader
    {
        SHADERS_PATH + "shadow/point.vert", 
        SHADERS_PATH + "shadow/point.geom", 
        SHADERS_PATH + "shadow/point.frag"         
    };    
    GLuint point_shadow_text = TEXTURE_MANAGER.generate_cube_texture_buffer(1024, 1024);    
    float near_plane = 0.1;
    float far_plane = 75.0;

    virtual void application() override
    {

        stbi_set_flip_vertically_on_load(true);
        
        teapot_obj.load_single_obj({"../resources/obj/teapot.obj"});
        teapot_model = glm::mat4(1.0);
        teapot_model = glm::translate(teapot_model, {0.0, 2.0, 0.0});
        floor_obj.load_single_obj({"../resources/obj/floor.obj"});
        floor_model = glm::mat4(1.0);
        floor_model = glm::scale(floor_model, {10.0, 10.0, 10.0});
        // pre compute
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        CAMERA.set_position({0.0, 3.0, 10.0});
        budf_lut.execute();
        equirect_pass.execute(_input_hdr);
        convolution_pass.execute(equirect_pass);
        prefilter_pass.execute(equirect_pass);
        glfwGetFramebufferSize(window, &_width, &_height);     
        // gbuffer set
        gbuffer_fb.bind();
        gbuffer_fb.create_render_object(_width, _height);
        gbtx_position = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, TEXTURE_2D_RGBA16F);
        gbuffer_fb.attach_color_texture(0, gbtx_position);
        gbtx_albdeo = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, TEXTURE_2D_RGBA);
        gbuffer_fb.attach_color_texture(1, gbtx_albdeo);
        gbtx_normal = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, TEXTURE_2D_RGBA16F);
        gbuffer_fb.attach_color_texture(2, gbtx_normal);
        gbtx_effects = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, TEXTURE_2D_RGB16F);
        gbuffer_fb.attach_color_texture(3, gbtx_effects);
        gbuffer_fb.active_draw_buffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
        gbuffer_fb.checkFramebufferStatus();
        gbuffer_fb.unbind();

        // gbuffer
        Material::set_samplers(gbuffer_sp, 0);
             
        // light
        light_fb.bind();
        light_fb.create_render_object(_width, _height);
        light_result_texture = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, TEXTURE_2D_RGBA16F);
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
        light_sp.set_sampler(8, "d_shadow_text");
        light_sp.set_sampler(9, "p_shadow_text");
        light_sp.set_uniform("d_light.color", direction_light.irradiance);
        light_sp.set_uniform("d_light.direction", std::get<DirectionalLight>(direction_light.detail).direction);

        light_sp.set_uniform("p_light.position", std::get<PointLight>(point_light.detail).position);
        light_sp.set_uniform("p_light.color", point_light.irradiance);
        light_sp.set_uniform("p_light.constant", std::get<PointLight>(point_light.detail).attenuation[0]);
        light_sp.set_uniform("p_light.linear", std::get<PointLight>(point_light.detail).attenuation[1]);
        light_sp.set_uniform("p_light.quadratic", std::get<PointLight>(point_light.detail).attenuation[2]);

        // postprocess
        _display_pass.set(_width, _height);
        _color_correction_pass.set(_width, _height);
        _depth24_debug.set(_width, _height);

        _fxaa_pass._enable = true;
        _fxaa_pass.set(_width, _height);
        _fxaa_pass._sp.use();
        _fxaa_pass._sp.set_uniform("frag_size", glm::vec2(1.0 / _width, 1.0 / _height));

        _motion_blur_pass._enable = false;
        _motion_blur_pass.set(_width, _height);
        _motion_blur_pass._sp.use();
        _motion_blur_pass._sp.set_sampler(0, "screenTexture");
        _motion_blur_pass._sp.set_sampler(1, "gEffects");

        // 计算阴影
        direction_shadow.begin();
        direction_shadow._sp->set_uniform("model", teapot_model);
        teapot_obj.render_elements();
        direction_shadow._sp->set_uniform("model", floor_model);
        floor_obj.render_elements();
        direction_shadow.end();

        // point_shadow.begin();
        // point_shadow._sp->set_uniform("model", teapot_model);
        // teapot_obj.render_elements();
        // point_shadow._sp->set_uniform("model", floor_model);
        // floor_obj.render_elements();
        // point_shadow.end();

        // point shadow debuug
        frame.bind();
        glEnable(GL_DEPTH_TEST);
        frame.attach_depth_texture_array(point_shadow_text);
        frame.set_draw_read(GL_NONE, GL_NONE);
        frame.unbind();
        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        auto lightPos = std::get<PointLight>(point_light.detail).position;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)1024 / (float)1024, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        glViewport(0, 0, 1024, 1024);
        frame.bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        point_depth_shader.use();
        for (unsigned int i = 0; i < 6; ++i)
        {
            point_depth_shader.set_uniform("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        point_depth_shader.set_uniform("far_plane", far_plane);
        point_depth_shader.set_uniform("position", lightPos);
        glCullFace(GL_FRONT);//改变面剔除以解决阴影悬浮问题
        point_depth_shader.set_uniform("model", teapot_model);
        teapot_obj.render_elements();
        point_depth_shader.set_uniform("model", floor_model);
        floor_obj.render_elements();        
        glCullFace(GL_BACK);//改变面剔除以解决阴影悬浮问题
        glBindFramebuffer(GL_FRAMEBUFFER, 0);         

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
        light_sp.set_uniform("fragment_size", glm::vec2(1.0 / _width, 1.0 / _height));
        light_sp.set_uniform("d_light_matrix", (glm::mat4)direction_shadow.get_light_matrix());
        light_sp.active_sampler(0, gbtx_position);
        light_sp.active_sampler(1, gbtx_albdeo);
        light_sp.active_sampler(2, gbtx_normal);
        light_sp.active_sampler(3, gbtx_effects);
        light_sp.active_sampler(4, convolution_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(5, prefilter_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(6, budf_lut);
        light_sp.active_sampler(7, equirect_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(8, direction_shadow);
        light_sp.active_sampler(9, point_shadow_text, GL_TEXTURE_CUBE_MAP);
        VertexArray::render_empty_va();     
        light_fb.unbind();     
    }

    void postprocess()
    {
        auto final = light_result_texture;
        if (_motion_blur_pass._enable)
        {
            _motion_blur_pass.execute({final, gbtx_effects});
            final = _motion_blur_pass;
        }
        _color_correction_pass.execute(final);
        if (_fxaa_pass._enable)
        {
            _fxaa_pass.execute(_color_correction_pass);
            final = _fxaa_pass;
        }
        _display_pass.render(final);
    }

    virtual void render_loop() override
    {
        geometry_render();
        light_render();
        postprocess();
        _display_pass.render(_fxaa_pass);
        // _depth24_debug.render(direction_shadow);
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