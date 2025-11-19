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

    ShaderProgram pbrShader
    {
        SHADERS_PATH + "render/pbr.vs",
        SHADERS_PATH + "render/pbr.fs" 
    };  
    
    ShaderProgram merge_sp
    {
        SHADERS_PATH + "render/merge.vert",
        SHADERS_PATH + "render/merge.frag" 
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

    FrameBuffer d_fb;
    GLuint d_position;
    GLuint d_normal;


    virtual void application() override
    {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        CAMERA.set_position({0.0, 0.0, 4.0});
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

        d_fb.bind();
        gbuffer_fb.create_render_object(scrWidth, scrHeight);
        d_position = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA16F);
        gbuffer_fb.attach_color_texture(0, d_position);
        d_normal = TEXTURE_MANAGER.generate_texture_buffer(scrWidth, scrHeight, TEXTURE_2D_RGBA16F);
        gbuffer_fb.attach_color_texture(1, d_normal);
        gbuffer_fb.active_draw_buffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
        d_fb.unbind();

        _debug_light_sp.use();
        _debug_light_sp.set_sampler(0, "s_position");
        _debug_light_sp.set_sampler(1, "s_normal");
        _debug_light_sp.set_uniform("d_light.color", glm::vec3(10.0, 10.0, 10.0));
        _debug_light_sp.set_uniform("d_light.direction", glm::vec3(1.0, 1.0, 1.0));
        _debug_light_sp.set_uniform("projection", get_projection());          


        // gbuffer
        gbuffer_sp.use();
        Material::set_samplers(gbuffer_sp, 0);
        // gbuffer_sp.set_sampler(0, "s_albedo");
        // gbuffer_sp.set_sampler(1, "s_normal");
        // gbuffer_sp.set_sampler(2, "s_roughness");
        // gbuffer_sp.set_sampler(3, "s_metalness");
        // gbuffer_sp.set_sampler(4, "s_ao");
      
             
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
        
        light_sp.set_uniform("d_light.color", glm::vec3(100.0, 100.0, 100.0));
        light_sp.set_uniform("d_light.direction", glm::vec3(1.0, 1.0, 1.0));

        // pbr
        pbrShader.use();
        pbrShader.set_uniform<int>("irradianceMap", 0);
        pbrShader.set_uniform<int>("prefilterMap", 1);
        pbrShader.set_uniform<int>("brdfLUT", 2);
        pbrShader.set_uniform<int>("albedoMap", 3);
        pbrShader.set_uniform<int>("normalMap", 4);
        pbrShader.set_uniform<int>("metallicMap", 5);
        pbrShader.set_uniform<int>("roughnessMap", 6);
        pbrShader.set_uniform<int>("aoMap", 7);        

        pbrShader.set_uniform("d_light.color", glm::vec3(10.0, 10.0, 10.0));
        pbrShader.set_uniform("d_light.direction", glm::vec3(1.0, 1.0, 1.0));
        pbrShader.set_uniform("projection", get_projection());

        merge_sp.use();
        merge_sp.set_uniform("d_light.color", glm::vec3(10.0, 10.0, 10.0));
        merge_sp.set_uniform("d_light.direction", glm::vec3(1.0, 1.0, 1.0));
        merge_sp.set_uniform("projection", get_projection());        


    }

    void debug_defferd()
    {
        d_fb.bind();
        update_viewport();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        _debug_gbuffer_sp.use();
        glm::mat4 model = glm::mat4(1.0f);
        _debug_gbuffer_sp.set_uniform("model", model);
        _debug_gbuffer_sp.set_uniform("normal_matrix", glm::transpose(glm::inverse(glm::mat3(model))));
        _debug_gbuffer_sp.set_uniform("proj_view_model", get_projection() * CAMERA.get_view_matrix() * model);
        Shape::render_sphere();
        d_fb.unbind();

        update_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);       
        glDepthMask(GL_FALSE); 
        _debug_light_sp.use();
        _debug_light_sp.active_sampler(0, d_position);
        _debug_light_sp.active_sampler(1, d_normal);
        _debug_light_sp.set_uniform("eye_position", CAMERA.get_position());
        VertexArray::render_empty_va();        
    }

    void merge_render()
    {
        update_viewport();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);           
        merge_sp.use();
        glm::mat4 model = glm::mat4(1.0f);
        merge_sp.set_uniform("model", model);
        merge_sp.set_uniform("normal_matrix", glm::transpose(glm::inverse(glm::mat3(model))));
        merge_sp.set_uniform("proj_view_model", get_projection() * CAMERA.get_view_matrix() * model);
        merge_sp.set_uniform("eye_position", CAMERA.get_position());
        Shape::render_sphere();        
    }

    void direct_render()
    {
        update_viewport();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);        
        pbrShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        pbrShader.set_uniform("view", CAMERA.get_view_matrix());
        pbrShader.set_uniform("camPos", CAMERA.get_position());
        pbrShader.set_uniform("model", model);
        pbrShader.set_uniform("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, convolution_pass);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_pass);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, budf_lut); 
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, rusted_iron._albedo);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, rusted_iron._normal);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, rusted_iron._metallic);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, rusted_iron._roughness);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, rusted_iron._ao);   
        Shape::render_sphere();        
        for (unsigned int i = 0; i < 4; ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            pbrShader.set_uniform("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrShader.set_uniform("lightColors[" + std::to_string(i) + "]", lightColors[i]);
            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            pbrShader.set_uniform("model", model);
            pbrShader.set_uniform("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            // Shape::render_sphere();
        }        
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
        glm::mat4 model(1.0);
        glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(model)));
        glm::mat4 proj_view_model = get_projection() * CAMERA.get_view_matrix() * model;
        gbuffer_sp.set_uniform("model", model);
        gbuffer_sp.set_uniform("normal_matrix", normal_matrix);
        gbuffer_sp.set_uniform("proj_view_model", proj_view_model);
        gbuffer_sp.set_uniform("prev_proj_view_model", prev_proj_view_model);
        prev_proj_view_model = proj_view_model;
        rusted_iron.active(0); 
        Shape::render_sphere();//some render draw call
        gbuffer_fb.unbind();
        // light
        update_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);       
        glDepthMask(GL_FALSE);          
        light_sp.use();
        light_sp.set_uniform("eye_position", CAMERA.get_position());
        light_sp.active_sampler(0, gbtx_position);
        light_sp.active_sampler(1, gbtx_albdeo);
        light_sp.active_sampler(2, gbtx_normal);
        light_sp.active_sampler(3, gbtx_effects);
        light_sp.active_sampler(4, convolution_pass);
        light_sp.active_sampler(5, prefilter_pass);
        light_sp.active_sampler(6, budf_lut);
        light_sp.active_sampler(7, equirect_pass);
        VertexArray::render_empty_va();        
    }

    virtual void render_loop() override
    {

        // direct_render();


        // merge_render();

        // debug_defferd();

        deffered_render();

        // // ready for forward render
        // int scrWidth, scrHeight;
        // glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        // glViewport(0, 0, scrWidth, scrHeight);          
        // glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer_fb);
        // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   
        // glBlitFramebuffer(0, 0, scrWidth, scrHeight, 0, 0, scrWidth, scrHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);        

        // _debug.render_texture(rusted_iron._normal);
        // _debug.render_texture(gbtx_normal);
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