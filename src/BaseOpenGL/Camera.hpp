#pragma once
#include "utility.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 45.0f;

#define CAMERA Camera::getInstance()
class Camera
{
    SINGLETON(Camera);
    glm::vec3 _position{0.0f, 0.0f, 3.0f};
    glm::vec3 _front{0.0f, 0.0f, -1.0f};
    glm::vec3 _up{0.0f, 1.0f, 0.0f};
    glm::vec3 _right;
    glm::vec3 _world_up{0.0f, 1.0f, 0.0f};
    float _yaw{-90.0f};
    float _pitch{0.0f};
    float _move_speed{2.5f};
    float _rotate_speed{45.0f};
    float _sensitivity{0.1f};
    float _zoom{45.0f};
public:
    void init() 
    {
        update();
    }
    void set_position(glm::vec3 position) 
    { 
        _position = position; update(); 
    } 
    glm::vec3 get_position() const { return _position; }
    glm::vec3 get_front() const { return _front; }
    float get_zoom() const {return _zoom; }
    void process_keyboard_input(unsigned int key, float delta_time)
    {
        switch (key) 
        {
            case GLFW_KEY_W:
            {
                _position += _front * _move_speed * delta_time;
                break;
            }
            case GLFW_KEY_S:
            {
                _position -= _front * _move_speed * delta_time;
                break;
            }
            case GLFW_KEY_A:
            {
                _position -= _right * _move_speed * delta_time;
                break;
            }
            case GLFW_KEY_D:
            {
                _position += _right * _move_speed * delta_time;
                break;
            }
            case GLFW_KEY_SPACE:
            {
                _position += _up * _move_speed * delta_time;
                break;
            }  
            case GLFW_KEY_LEFT_SHIFT:
            {
                _position -= _up * _move_speed * delta_time;
                break;
            }        
        }
    }
    void process_mouse_scroll(float yoffset)
    {
        _zoom -= (float)yoffset;
        if (_zoom < 1.0f)
        {
            _zoom = 1.0f;
        }
        if (_zoom > 45.0f)
        {
            _zoom = 45.0f;
        }
    }
    void process_mouse_movement(float xoffset, float yoffset)
    {
        xoffset *= _sensitivity;
        yoffset *= _sensitivity;
        _yaw += xoffset;
        _pitch += yoffset;
        if (_pitch > 89.0f)
        {
            _pitch = 89.0f;
        }
        if (_pitch < -89.0f)
        {
            _pitch = -89.0f;
        }
        update();
    }
    glm::mat4 get_view_matrix()
    {
        return glm::lookAt(_position, _position + _front, _up);
    }
private:
    void update()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front = glm::normalize(front);
        _right = glm::normalize(glm::cross(_front, _world_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        _up = glm::normalize(glm::cross(_right, _front));
    }
};

