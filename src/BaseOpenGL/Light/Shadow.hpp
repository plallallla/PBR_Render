#pragma once
#include <glad/glad.h>
#include <memory>

#include "Light.hpp"
#include "FrameBuffer.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"

class Shadow
{
    FrameBuffer _fb;
    light_type _type;
    GLuint _width;
    GLuint _height;
    glm::vec3 light_geometry;
    inline static std::shared_ptr<ShaderProgram> get_directional_sp() 
    {
        static auto instance = std::make_shared<ShaderProgram>
        (
            SHADERS_PATH + "shadow/directional.vert",
            SHADERS_PATH + "post_process/directional.frag"
        );
        return instance;
    }

    inline static std::shared_ptr<ShaderProgram> get_point_sp() 
    {
        static auto instance = std::make_shared<ShaderProgram>
        (
            SHADERS_PATH + "shadow/point.vert", 
            SHADERS_PATH + "shadow/point.geom", 
            SHADERS_PATH + "post_process/point.frag" 
        );
        return instance;
    }

public:
    HAS_RESULT;
    std::shared_ptr<ShaderProgram> _sp;
    Shadow(const Light& l, GLuint width, GLuint height) : _type{l.type}, _width{width}, _height{height}
    {
        _fb.bind();
        if (l.type == light_type::directional)
        {
            _sp = get_directional_sp();
            light_geometry = std::get<DirectionalLight>(l.detail).direction;
            _result = TEXTURE_MANAGER.generate_texture_buffer(width, height, TEXTURE_2D_DEPTH);
        }
        else if (l.type == light_type::point)
        {
            _sp = get_point_sp();
            light_geometry = std::get<PointLight>(l.detail).position;
            _result = TEXTURE_MANAGER.generate_cube_texture_buffer(width, height, TEXTURE_CUBE_DEPTH);
        }
        _fb.attach_depth_texture(_result);
        _fb.set_draw_read(GL_NONE, GL_NONE);        
        _fb.unbind();
    }
    
    void begin()
    {
        glViewport(0, 0, _width, _height);
        _fb.bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        _sp->use();
        if (_type == light_type::directional)
        {
            float scene_radius = 30.0f; 
            glm::vec3 scene_center = glm::vec3(0.0, 0.0, 0.0);
            glm::vec3 direction = glm::normalize(light_geometry);
            // 防止 up 向量与 lightDir 共线（如正午太阳）
            glm::vec3 up = std::abs(direction.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);            
            // 固定包围球来模拟平行光位置
            glm::vec3 light_pos = scene_center - direction * (scene_radius * 2.0f);
            glm::mat4 view = glm::lookAt(light_pos, scene_center, up);
            glm::mat4 projection = glm::ortho(-scene_radius, scene_radius, -scene_radius, scene_radius, .1f, scene_radius * 4.0f);
            _sp->set_uniform("projection_view", projection * view);
        }
        else if (_type == light_type::point)
        {
            glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)_width / (float)_height, .1f, 25.0f);
            _sp->set_uniform("shadowMatrices[" + std::to_string(0) + "]", projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            _sp->set_uniform("shadowMatrices[" + std::to_string(1) + "]", projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            _sp->set_uniform("shadowMatrices[" + std::to_string(2) + "]", projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
            _sp->set_uniform("shadowMatrices[" + std::to_string(3) + "]", projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
            _sp->set_uniform("shadowMatrices[" + std::to_string(4) + "]", projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            _sp->set_uniform("shadowMatrices[" + std::to_string(5) + "]", projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            _sp->set_uniform("far_plane", 25.0f);
            _sp->set_uniform("position", light_geometry);
        }
    }

    void end()
    {
        glCullFace(GL_BACK);//改变面剔除以解决阴影悬浮问题
        glBindFramebuffer(GL_FRAMEBUFFER, 0);            
    }

};