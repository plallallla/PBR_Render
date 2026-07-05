add_executable(PBR_Render main.cpp)

target_link_libraries(PBR_Render PRIVATE
    BaseOpenGL
    glfw
    glad::glad
    assimp::assimp
    spdlog::spdlog_header_only
    imgui::imgui
)

target_include_directories(PBR_Render PRIVATE ${Stb_INCLUDE_DIR})

target_compile_definitions(PBR_Render PRIVATE
    TEXTURE_PATH=std::string{"${CMAKE_SOURCE_DIR}/resources/textures/"}
    OBJ_PATH=std::string{"${CMAKE_SOURCE_DIR}/resources/obj/"}
    SHADERS_PATH=std::string{"${CMAKE_SOURCE_DIR}/resources/shaders/"}
)
