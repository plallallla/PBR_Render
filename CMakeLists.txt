cmake_minimum_required(VERSION 3.10.0)

project(PBR_Render VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src/BaseOpenGL)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)


add_executable(PBR_Render src/main.cpp)

target_compile_definitions(PBR_Render PRIVATE TEXTURE_PATH=std::string{"../resources/textures/"})
target_compile_definitions(PBR_Render PRIVATE SHADERS_PATH="1.2.3")

target_link_libraries(PBR_Render PRIVATE glfw glad::glad assimp::assimp spdlog::spdlog_header_only imgui::imgui)
target_include_directories(PBR_Render PRIVATE ${Stb_INCLUDE_DIR})