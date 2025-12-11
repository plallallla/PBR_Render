#include "Camera.hpp"
#include "FrameBuffer.hpp"
#include "GLWidget.hpp"
#include "PrecomputedRender.hpp"
#include "PostprocessRender.hpp"
#include "ShaderProgram.hpp"
#include "Shadow.hpp"
#include "Shape.hpp"
#include "SkyboxRender.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "TextureAttributes.hpp"
#include "VertexArray.hpp"
#include "utility.hpp"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <imgui.h>
#include <vector>

#include "Model.hpp"

class PBR_render : public GLWidget
{
    /* 纹理材质 */
    std::vector<Material> _materials
    {
        {TEXTURE_PATH + "pbr/rusted_iron"},
        {TEXTURE_PATH + "pbr/gold"},
        {TEXTURE_PATH + "pbr/plastic"},
        {TEXTURE_PATH + "pbr/wall"},
    };
    Material woodfloor{TEXTURE_PATH + "pbr/woodfloor"};
    int _material_index{0};

    // 天空盒渲染pass
    GLuint _input_hdr = TEXTURE_MANAGER.auto_load_texture(TEXTURE_PATH + "hdr/newport_loft.hdr");
    SkyboxRender _skybox;

    /* 预计算 */
    BRDF_LUT budf_lut;
    EquirectConvertRender equirect_pass;
    ConvolutionIBLRender convolution_pass;
    PrefilterIBLRender prefilter_pass;

    /* 光照计算资源 */
    FrameBuffer light_fb;
    GLuint light_result_texture;
    ShaderProgram light_sp
    {
        SHADERS_PATH + "render/light.vert",
        SHADERS_PATH + "render/light.frag" 
    };
    ShaderProgram lighting_obj_sp
    {
        SHADERS_PATH + "forward/lighting_obj.vert",
        SHADERS_PATH + "forward/lighting_obj.frag"
    };


    /* light & shadow */
    // 用于光线透视矩阵
    float near_plane = 0.1;
    float far_plane = 75.0;   
    // light data
    Light direction_light
    {
        light_type::directional,
        glm::vec3(10.0, 10.0, 10.0),
        DirectionalLight{glm::vec3(1.0, 1.0, 1.0)}
    }; 
    Light point_light
    {
        light_type::point,
        glm::vec3(30.0, 30.0, 30.0),
        PointLight{glm::vec3(0.0f, 5.0f, 0.0f)/*position*/, {0.0, 1.0, 0.1}}
    };
    // shadow map
    DirectionalShadow direction_shadow{2048};    
    PointShadow point_shadow{2048};      

    /* 几何资源 */
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

    /* 后处理资源 */
    // regular display
    PostprocessRender _display_pass{ SHADERS_PATH + "post_process/display.frag" };
    // shadow debug
    PostprocessRender _depth24_debug{ SHADERS_PATH + "post_process/depth24_debug.frag" };
    // color correction <- tone map + hdr
    PostprocessRender _color_correction_pass{ SHADERS_PATH + "post_process/color_correction.frag" };
    // aa
    PostprocessRender _fxaa_pass{ SHADERS_PATH + "post_process/fxaa.frag" };
    // motion blur
    PostprocessRender _motion_blur_pass{ SHADERS_PATH + "post_process/motion_blur.frag" };
    // bloom blur
    PostprocessRender _h_bloom_blur_pass{ SHADERS_PATH + "post_process/horizontal_bloom_blur.frag" };
    PostprocessRender _v_bloom_blur_pass{ SHADERS_PATH + "post_process/vertical_bloom_blur.frag" };
    PostprocessRender _bright_extraction_pass{ SHADERS_PATH + "post_process/bright_extraction.frag" };
    PostprocessRender _mixture_pass{ SHADERS_PATH + "post_process/mixture.frag" };
    float _bloom_bright_threshold{1.0};
    // sao
    PostprocessRender _sao_compute_pass{ SHADERS_PATH + "post_process/sao_compute.frag" };
    PostprocessRender _sao_blur_pass{ SHADERS_PATH + "post_process/sao_blur.frag" };

    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 prev_view;
    glm::mat3 normal_matrix;

    // 模型数据
    Model teapot_obj;
    glm::mat4 teapot_model;
    Model floor_obj;
    glm::mat4 floor_model;

    virtual void application() override
    {

        stbi_set_flip_vertically_on_load(true);
        
        // scene
        teapot_obj.load_single_obj({"../resources/obj/teapot.obj"});
        teapot_model = glm::mat4(1.0);
        teapot_model = glm::translate(teapot_model, {0.0, 2.0, 0.0});
        floor_obj.load_single_obj({"../resources/obj/floor.obj"});
        floor_model = glm::mat4(1.0);
        floor_model = glm::scale(floor_model, {50.0, 50.0, 50.0});

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
        auto pos_att = TEXTURE_2D_RGBA16F;
        pos_att._mipmap = true;// 开启mipmap用于AlchemyAO
        gbtx_position = TEXTURE_MANAGER.generate_texture_buffer(_width, _height, pos_att);
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

        auto frag_size = glm::vec2(1.0 / _width, 1.0 / _height);
        // common postprocess set
        _display_pass.set(_width, _height);
        _color_correction_pass.set(_width, _height);
        _depth24_debug.set(_width, _height);
        // fxaa
        _fxaa_pass._enable = true;
        _fxaa_pass.set(_width, _height);
        _fxaa_pass._sp.use();
        _fxaa_pass._sp.set_uniform("frag_size", frag_size);
        // motion blur
        _motion_blur_pass._enable = false;
        _motion_blur_pass.set(_width, _height);
        _motion_blur_pass._sp.use();
        _motion_blur_pass._sp.set_sampler(0, "screenTexture");
        _motion_blur_pass._sp.set_sampler(1, "gEffects");
        _motion_blur_pass._sp.set_uniform("frag_size", frag_size);
        // bloom blur
        _h_bloom_blur_pass.set(_width, _height);
        _h_bloom_blur_pass._enable = true;
        _v_bloom_blur_pass.set(_width, _height);
        _v_bloom_blur_pass._enable = true;
        _bright_extraction_pass.set(_width, _height);
        _bright_extraction_pass._enable = true;
        _mixture_pass.set(_width, _height);
        _mixture_pass._enable = true;
        _mixture_pass._sp.use();
        _mixture_pass._sp.set_sampler(0, "screenTexture1");
        _mixture_pass._sp.set_sampler(1, "screenTexture2");
        // sao
        _sao_compute_pass._enable = true;
        _sao_compute_pass.set(_width, _height, TEXTURE_2D_FLOAT);
        _sao_compute_pass._sp.use();
        _sao_compute_pass._sp.set_sampler(0, "s_position");
        _sao_compute_pass._sp.set_sampler(1, "s_normal");

        _sao_blur_pass.set(_width, _height);
        _sao_blur_pass._sp.use();
        _sao_blur_pass._sp.set_uniform("frag_size", frag_size);
        _sao_blur_pass._sp.set_sampler(0, "screenTexture");
        _sao_blur_pass._sp.set_sampler(1, "sao_result");

        projection = get_projection();
        view = CAMERA.get_view_matrix(); 
        prev_view = view;

    }

    void shadow_compute()
    {
        // 计算方向阴影
        direction_shadow.begin(std::get<DirectionalLight>(direction_light.detail).direction);
        direction_shadow._sp.set_uniform("model", teapot_model);
        teapot_obj.render_elements();
        direction_shadow._sp.set_uniform("model", floor_model);
        floor_obj.render_elements();
        direction_shadow.end();
        // 计算点阴影
        point_shadow.begin(std::get<PointLight>(point_light.detail).position);
        point_shadow._sp.set_uniform("model", teapot_model);
        teapot_obj.render_elements();
        point_shadow._sp.set_uniform("model", floor_model);
        floor_obj.render_elements();
        point_shadow.end();
    }

    void render_object(Model& m, const glm::mat4 model, const Material& material)
    {
        gbuffer_sp.set_uniform("model", model);
        gbuffer_sp.set_uniform("eye_position", CAMERA.get_position());
        normal_matrix = glm::mat3(glm::transpose(glm::inverse(model)));
        gbuffer_sp.set_uniform("normal_matrix", normal_matrix);
        gbuffer_sp.set_uniform("proj_view_model", projection * view * model);
        gbuffer_sp.set_uniform("prev_proj_view_model", projection * prev_view * model);
        material.active(0);
        m.render_elements();        
    }    

    void geometry_render()
    {
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
        render_object(teapot_obj, teapot_model, _materials[_material_index]);
        render_object(floor_obj, floor_model, woodfloor);
        gbuffer_fb.unbind();
        prev_view = view;
    }    

    void light_render()
    {
        light_fb.bind();
        update_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        light_sp.use();
        // geometry
        light_sp.set_uniform("eye_position", CAMERA.get_position());
        light_sp.set_uniform("cube_uv_trans", glm::inverse(glm::mat4(glm::mat3(view))) * glm::inverse(projection));        
        light_sp.set_uniform("fragment_size", glm::vec2(1.0 / _width, 1.0 / _height));
        light_sp.set_uniform("d_light_matrix", (glm::mat4)direction_shadow.get_light_matrix());
        // light
        light_sp.set_uniform("d_light.color", direction_light.irradiance);
        light_sp.set_uniform("d_light.direction", std::get<DirectionalLight>(direction_light.detail).direction);
        light_sp.set_uniform("p_light.position", std::get<PointLight>(point_light.detail).position);
        light_sp.set_uniform("p_light.color", point_light.irradiance);
        light_sp.set_uniform("p_light.constant", std::get<PointLight>(point_light.detail).attenuation[0]);
        light_sp.set_uniform("p_light.linear", std::get<PointLight>(point_light.detail).attenuation[1]);
        light_sp.set_uniform("p_light.quadratic", std::get<PointLight>(point_light.detail).attenuation[2]);
        // sampler
        light_sp.active_sampler(0, gbtx_position);
        light_sp.active_sampler(1, gbtx_albdeo);
        light_sp.active_sampler(2, gbtx_normal);
        light_sp.active_sampler(3, gbtx_effects);
        light_sp.active_sampler(4, convolution_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(5, prefilter_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(6, budf_lut);
        light_sp.active_sampler(7, equirect_pass, GL_TEXTURE_CUBE_MAP);
        light_sp.active_sampler(8, direction_shadow);
        light_sp.active_sampler(9, point_shadow, GL_TEXTURE_CUBE_MAP);
        // render
        VertexArray::render_empty_va();     
        // forawrd render light object
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer_fb);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fb);
        glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        lighting_obj_sp.use();
        lighting_obj_sp.set_uniform("light_color", point_light.irradiance);
        glm::mat4 model(1.0);
        model = glm::translate(model, std::get<PointLight>(point_light.detail).position);
        model = glm::scale(model, {.2, .2, .2});
        lighting_obj_sp.set_uniform("trans", projection * view * model);
        Shape::render_sphere();
        light_fb.unbind();
    }

    // 颜色未校正 基于物理颜色处理
    GLuint physical_postprocess(GLuint input)
    {
        if (_sao_compute_pass._enable)
        {
            _sao_compute_pass._sp.use();
            _sao_compute_pass._sp.set_uniform("position_transform", view);
            _sao_compute_pass._sp.set_uniform("normal_transform", normal_matrix);
            _sao_compute_pass._sp.set_uniform("eye_position", CAMERA.get_position());
            _sao_compute_pass.execute({gbtx_position, gbtx_normal});
            _sao_blur_pass.execute({input, _sao_compute_pass});
            input = _sao_blur_pass;
        }
        if (_bright_extraction_pass._enable)
        {
            _bright_extraction_pass._sp.use();
            _bright_extraction_pass._sp.set_uniform<float>("threshold", _bloom_bright_threshold);
            _bright_extraction_pass.execute(input);
            GLuint bloom_blur_target = _bright_extraction_pass;    
            if (_h_bloom_blur_pass._enable && _v_bloom_blur_pass._enable) 
            {
                for (int i = 0; i < 10; i++)
                {
                    _h_bloom_blur_pass.execute(bloom_blur_target);
                    _v_bloom_blur_pass.execute(_h_bloom_blur_pass);
                    bloom_blur_target = (GLuint)_v_bloom_blur_pass;
                }
            }
            _mixture_pass.execute({input, bloom_blur_target});
            input = _mixture_pass;
        }
        return input;
    }

    // 颜色已校正 基于视觉颜色处理
    GLuint visiual_postprocess(GLuint input)
    {
        if (_motion_blur_pass._enable)
        {
            _motion_blur_pass.execute({input, gbtx_effects});
            input = _motion_blur_pass;
        }        
        if (_fxaa_pass._enable)
        {
            _fxaa_pass.execute(input);
            input = _fxaa_pass;
        }
        return input;
    }    

    GLuint postprocess(GLuint input)
    {
        return physical_postprocess(input);
        _color_correction_pass.execute(physical_postprocess(input));
        return visiual_postprocess(_color_correction_pass);
    }

    virtual void render_loop() override
    {
        shadow_compute();
        geometry_render();
        light_render();
        _display_pass.render(postprocess(light_result_texture));
    }

    virtual void gui_operation() override
    {
        DirectionalLight* dl = std::get_if<DirectionalLight>(&direction_light.detail);
        ImGui::SliderFloat(u8"direction light.x", &dl->direction.x, -10.0, 10.0);
        ImGui::SliderFloat(u8"direction light.z", &dl->direction.z, -10.0, 10.0);
        PointLight* pl = std::get_if<PointLight>(&point_light.detail);
        ImGui::SliderFloat(u8"point light.x", &pl->position.x, -10.0, 10.0);
        ImGui::SliderFloat(u8"point light.y", &pl->position.y, 0.05, 10.0);
        ImGui::SliderFloat(u8"point light.z", &pl->position.z, -10.0, 10.0);
        ImGui::Checkbox(u8"sao", &_sao_compute_pass._enable);
        ImGui::Checkbox(u8"bloom blur", &_bright_extraction_pass._enable);
        if (_bright_extraction_pass._enable)
        {
            ImGui::SliderFloat(u8"bloom bright threshold", &_bloom_bright_threshold, 0.5, 10.0);
        }
        ImGui::Checkbox(u8"motion blur", &_motion_blur_pass._enable);
        ImGui::Checkbox(u8"fxaa", &_fxaa_pass._enable);
        const char* items[] = { u8"rusted iron", u8"gold", u8"plastic", u8"wall" };
        ImGui::Combo("Material", &_material_index, items, IM_ARRAYSIZE(items));
    }

public:
    PBR_render(int width, int height, std::string_view title) : GLWidget(width,height,title,true) 
    {
    }
};

int main()
{
    PBR_render pbr_render_widget{900, 800, "pbr_render"};
    pbr_render_widget.render();
    return 0;
}