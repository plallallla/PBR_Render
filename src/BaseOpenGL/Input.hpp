/**
 * @file Input.hpp
 * @author pnghfng
 * @brief 处理鼠标键盘等操作
 * @date 2025-10-18
 * 
 */
#pragma once
#include "Camera.hpp"

#define INPUT Input::getInstance()
struct Input
{
    float _last_x;
    float _last_y;
    bool _first = true;
    float _last_frame_ts;
    float _delta_time;
    bool _debug{true};
    void init(unsigned int width, unsigned int height)
    {
        _last_x = width / 2.0f;
        _last_y = height / 2.0f;
    }
    float get_current_time()
    {
        return static_cast<float>(glfwGetTime());
    }
    void update_time()
    {
        float current = get_current_time();
        _delta_time = current - _last_frame_ts;
        _last_frame_ts = current;
    }
    SINGLETON(Input);
};

inline void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!INPUT._debug)
    {
        return;
    }    
    CAMERA.process_mouse_scroll(static_cast<float>(yoffset));
}

inline void mouse_move_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!INPUT._debug)
    {
        return;
    }
    int rightButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    int leftButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (INPUT._first)
    {
        INPUT._last_x = xpos;
        INPUT._last_y = ypos;
        INPUT._first = false;
    }
    float xoffset = xpos - INPUT._last_x;
    float yoffset = INPUT._last_y - ypos; // reversed since y-coordinates go from bottom to top
    INPUT._last_x = xpos;
    INPUT._last_y = ypos;
    if (rightButtonState != GLFW_PRESS && leftButtonState != GLFW_PRESS)
    {
        return;
    }
    CAMERA.process_mouse_movement(xoffset, yoffset);
}

inline void keyboard_input_callback(GLFWwindow *window)
{
    if (!INPUT._debug)
    {
        return;
    }    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_W, INPUT._delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_A, INPUT._delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_S, INPUT._delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_D, INPUT._delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_Q, INPUT._delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_E, INPUT._delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_SPACE, INPUT._delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        CAMERA.process_keyboard_input(GLFW_KEY_LEFT_SHIFT, INPUT._delta_time);
    }
}

inline void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (!INPUT._debug)
    {
        return;
    }    
    glViewport(0, 0, width, height);
}