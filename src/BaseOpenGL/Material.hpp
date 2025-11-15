#include <glad/glad.h>
#include "Texture.hpp"

struct Material
{
    GLuint _albedo;
    GLuint _normal;
    GLuint _metallic;
    GLuint _roughness;
    GLuint _ao;

    Material(std::string_view dir)
    {
        _albedo = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/albedo.png");
        _normal = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/normal.png");
        _metallic = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/metallic.png");
        _roughness = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/roughness.png");
        _ao = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/ao.png");
    }

    void active(GLenum offset)
    {
        glActiveTexture(GL_TEXTURE0 + offset);
        glBindTexture(GL_TEXTURE_2D, _albedo);
        glActiveTexture(GL_TEXTURE1 + offset);
        glBindTexture(GL_TEXTURE_2D, _normal);
        glActiveTexture(GL_TEXTURE2 + offset);
        glBindTexture(GL_TEXTURE_2D, _metallic);
        glActiveTexture(GL_TEXTURE3 + offset);
        glBindTexture(GL_TEXTURE_2D, _roughness);
        glActiveTexture(GL_TEXTURE4 + offset);
        glBindTexture(GL_TEXTURE_2D, _ao);        
    }

};