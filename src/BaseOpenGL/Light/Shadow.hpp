#pragma once
#include <glad/glad.h>
#include <memory>

#include "Light.hpp"
#include "FrameBuffer.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include "utility.hpp"

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
            SHADERS_PATH + "shadow/directional.frag"
        );
        return instance;
    }

    inline static std::shared_ptr<ShaderProgram> get_point_sp() 
    {
        static auto instance = std::make_shared<ShaderProgram>
        (
            SHADERS_PATH + "shadow/point.vert", 
            SHADERS_PATH + "shadow/point.geom", 
            SHADERS_PATH + "shadow/point.frag" 
        );
        return instance;
    }
    std::array<glm::mat4, 6> projection_view;

public:
    HAS_RESULT;
    std::shared_ptr<ShaderProgram> _sp;
    inline glm::mat4 get_light_matrix() const { return projection_view[0]; }
    inline std::array<glm::mat4, 6> get_light_matrixs() const { return projection_view; }
    Shadow(const Light& l, GLuint width, GLuint height) : _type{l.type}, _width{width}, _height{height}
    {
        projection_view.fill(glm::mat4(1.0));
        _fb.bind();
        if (l.type == light_type::directional)
        {
            _result = TEXTURE_MANAGER.generate_texture_buffer(width, height, TEXTURE_2D_DEPTH);
            _fb.attach_depth_texture(_result);
        }
        else if (l.type == light_type::point)
        {
            _result = TEXTURE_MANAGER.generate_cube_texture_buffer(width, height, TEXTURE_CUBE_DEPTH);
            _fb.attach_depth_texture_array(_result);
        }
        update_light(l);
        _fb.unbind();
    }

    void update_light(const Light& l)
    {
        if (l.type == light_type::directional)
        {
            _sp = get_directional_sp();
            light_geometry = std::get<DirectionalLight>(l.detail).direction;
            light_geometry.x *= -1.0;
            light_geometry.y *= -1.0;
            light_geometry.z *= -1.0;
            float scene_radius = 15.0f; 
            glm::vec3 scene_center = glm::vec3(0.0, 0.0, 0.0);
            glm::vec3 direction = glm::normalize(light_geometry);
            // 防止 up 向量与 lightDir 共线（如正午太阳）
            glm::vec3 up = std::abs(direction.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);            
            // 固定包围球来模拟平行光位置
            glm::vec3 light_pos = scene_center - direction * (scene_radius * 2.0f);
            glm::mat4 view = glm::lookAt(light_pos, scene_center, up);
            glm::mat4 projection = glm::ortho(-scene_radius, scene_radius, -scene_radius, scene_radius, .1f, scene_radius * 5.0f);
            projection_view[0] = projection * view;
        }
        else if (l.type == light_type::point)
        {
            _sp = get_point_sp();
            light_geometry = std::get<PointLight>(l.detail).position;
            glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)_width / (float)_height, .1f, 25.0f);
            projection_view[0] = projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            projection_view[1] = projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            projection_view[2] = projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            projection_view[3] = projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
            projection_view[4] = projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            projection_view[5] = projection * glm::lookAt(light_geometry, light_geometry + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));            
        }  
    }    
    
    void begin()
    {
        _fb.bind();
        _fb.set_draw_read(GL_NONE, GL_NONE);
        glViewport(0, 0, _width, _height);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        _sp->use();
        if (_type == light_type::directional)
        {
            _sp->set_uniform("projection_view", projection_view[0]);
        }
        else if (_type == light_type::point)
        {
            for (int i = 0; i < 6; i++)
            {
                _sp->set_uniform("shadowMatrices[" + std::to_string(i) + "]", projection_view[i]);
            }
            _sp->set_uniform("far_plane", 75.0f);
            _sp->set_uniform("position", light_geometry);
        }
    }

    void end()
    {
        glCullFace(GL_BACK);//改变面剔除以解决阴影悬浮问题
        glBindFramebuffer(GL_FRAMEBUFFER, 0);            
    }

};

class PointShadow
{
    FrameBuffer _fb;
    GLuint _width;
    GLuint _height;
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
    void update_texture_size(GLuint width, GLuint height)
    {
        _width = width;
        _height = height;
        if (_result)
        {
            TEXTURE_MANAGER.delete_texture(_result);
        }
        _result = TEXTURE_MANAGER.generate_cube_texture_buffer(_width, _height, TEXTURE_CUBE_DEPTH);
        _fb.bind();
        _fb.attach_depth_texture_array(_result);
        _fb.set_draw_read(GL_NONE, GL_NONE);
        _fb.unbind();
    }
    void update_light_position(glm::vec3 light_position)
    {
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)_width / (float)_height, _near, _far);
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
        if (_last_position != light_position)
        {
            update_light_position(light_position);
        }
        glViewport(0, 0, _width, _height);
        _fb.bind();
        _sp.use();
        glClear(GL_DEPTH_BUFFER_BIT);
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