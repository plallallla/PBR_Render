/**
 * @file GLWidget.hpp
 * @author pnghfng
 * @brief 封装了一些glad与glfw的初始化操作,增加imgui
 * @date 2025-10-18
 * 
 */
#pragma once
#include "Input.hpp"
#include <mutex>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class GLWidget
{
    /**
     * @brief 应用程序函数
     * 
     */
    virtual void application() = 0;
    /**
     * @brief 在渲染循环中执行
     * 
     */
    virtual void render_loop() = 0;
    /**
     * @brief gui操作
     * 
     */
    virtual void gui_operation() {}

protected:
    GLFWwindow* window{nullptr};
    int _width;
    int _height;
    int _debug_width{300};
    bool _gui{ false };

    inline glm::mat4 get_projection()
    {
        return glm::perspective(glm::radians(CAMERA.get_zoom()), (float)_width / (float)_height, 0.1f, 100.0f);
    }

public:
    GLWidget(int width, int height, std::string_view title, bool gui = false) : _width(width), _height(height), _gui(gui)
    {
        static std::once_flag gloable_init;
        std::call_once(gloable_init, [&] () 
        {
            glfwInit();
        });
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        window = glfwCreateWindow(width, height, title.data(), NULL, NULL);
        if (!window)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_move_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
        CAMERA.init();
        INPUT.init(width, height);
        // 初始化 ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        // 设置样式（可选：Dark / Light）
        ImGui::StyleColorsDark();

        // 初始化平台和渲染后端
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 150"); // macOS 推荐指定 GLSL 版本         
    }
    
    void update_viewport()
    {
        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        glViewport(0, 0, scrWidth, scrHeight);          
    }    

    void render()
    {
        application();
        while (!glfwWindowShouldClose(window))
        {
            keyboard_input_callback(window);
            INPUT.update_time();
            render_loop();
            if (_gui)
            {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                ImGui::Begin("debug", nullptr, ImGuiWindowFlags_AlwaysUseWindowPadding 
                    | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                INPUT._debug = ImGui::IsWindowCollapsed();
                ImGui::SetWindowPos(ImVec2(0, 0));           
                ImGui::SetWindowSize(ImVec2(_debug_width, _height));           
                gui_operation();
                ImGui::End();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());                
            }
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    ~GLWidget()
    {
        if (window)
        {
            LOG.info("glfwTerminate");
            glfwTerminate();
        }
    }
};