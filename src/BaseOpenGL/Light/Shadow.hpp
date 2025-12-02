#pragma once
#include <glad/glad.h>
#include "Light.hpp"
#include "FrameBuffer.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include "utility.hpp"

class DirectionalShadow
{
    FrameBuffer _fb;
    GLuint _size;
    glm::vec3 _last_direction; 
    glm::mat4 _light_matrix;
    
public:
    HAS_RESULT;
    ShaderProgram _sp
    {
        SHADERS_PATH + "shadow/directional.vert",
        SHADERS_PATH + "shadow/directional.frag"        
    };
    DirectionalShadow(GLuint size)
    {
        _size = size;
        if (_result) TEXTURE_MANAGER.delete_texture(_result);
        _result = TEXTURE_MANAGER.generate_texture_buffer(_size, _size, TEXTURE_2D_DEPTH);
        _fb.bind();
        _fb.attach_depth_texture(_result);
        _fb.set_draw_read(GL_NONE, GL_NONE);
        _fb.unbind();
    }
    void update_light_direction(glm::vec3 light_direction)
    {
        _last_direction = light_direction;
        light_direction.x *= -1.0;
        light_direction.y *= -1.0;
        light_direction.z *= -1.0;
        float scene_radius = 15.0f; 
        glm::vec3 scene_center = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 direction = glm::normalize(light_direction);
        // 防止 up 向量与 lightDir 共线（如正午太阳）
        glm::vec3 up = std::abs(direction.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);            
        // 固定包围球来模拟平行光位置
        glm::vec3 light_pos = scene_center - direction * (scene_radius * 2.0f);
        glm::mat4 view = glm::lookAt(light_pos, scene_center, up);
        glm::mat4 projection = glm::ortho(-scene_radius, scene_radius, -scene_radius, scene_radius, .1f, scene_radius * 5.0f);
        _light_matrix = projection * view;        
    }
    void begin(glm::vec3 light_direction)
    {
        if (_last_direction != light_direction) update_light_direction(light_direction);
        _fb.bind();
        _fb.set_draw_read(GL_NONE, GL_NONE);
        glViewport(0, 0, _size, _size);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        _sp.use();
        _sp.set_uniform("projection_view", _light_matrix);
    }
    void end()
    {
        glCullFace(GL_BACK);//改变面剔除以解决阴影悬浮问题
        glBindFramebuffer(GL_FRAMEBUFFER, 0);               
    }
    glm::mat4 get_light_matrix() const { return _light_matrix; }
};


class PointShadow
{
    FrameBuffer _fb;
    GLuint _size;
    float _near{.1f};
    float _far{75.f};
    glm::vec3 _last_position;
    std::array<glm::mat4, 6> _light_matrix;
public:
    HAS_RESULT;
    ShaderProgram _sp
    {
        SHADERS_PATH + "shadow/point.vert", 
        SHADERS_PATH + "shadow/point.geom", 
        SHADERS_PATH + "shadow/point.frag"        
    };
    PointShadow(GLuint size)
    {
        _size = size;
        if (_result) TEXTURE_MANAGER.delete_texture(_result);
        _result = TEXTURE_MANAGER.generate_cube_texture_buffer(_size, _size, TEXTURE_CUBE_DEPTH);
        _fb.bind();
        _fb.attach_depth_texture_array(_result);
        _fb.set_draw_read(GL_NONE, GL_NONE);
        _fb.unbind();
    }
    void update_light_position(glm::vec3 light_position)
    {
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, _near, _far);
        _light_matrix[0] = projection * glm::lookAt(light_position, light_position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        _light_matrix[1] = projection * glm::lookAt(light_position, light_position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        _light_matrix[2] = projection * glm::lookAt(light_position, light_position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        _light_matrix[3] = projection * glm::lookAt(light_position, light_position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        _light_matrix[4] = projection * glm::lookAt(light_position, light_position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        _light_matrix[5] = projection * glm::lookAt(light_position, light_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        _last_position = light_position;
    }
    void begin(glm::vec3 light_position)
    {
        if (_last_position != light_position) update_light_position(light_position);
        glViewport(0, 0, _size, _size);
        _fb.bind();
        _sp.use();
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_TRUE);
        for (unsigned int i = 0; i < 6; ++i)
        {
            _sp.set_uniform("shadowMatrices[" + std::to_string(i) + "]", _light_matrix[i]);
        }
        _sp.set_uniform("far_plane", _far);
        _sp.set_uniform("position", light_position);
        glCullFace(GL_FRONT);//改变面剔除以解决阴影悬浮问题
    }
    void end()
    {
        glCullFace(GL_BACK);//改变面剔除以解决阴影悬浮问题
        glBindFramebuffer(GL_FRAMEBUFFER, 0);               
    }
};