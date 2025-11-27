#pragma once
#include <glad/glad.h>
#include "ShaderProgram.hpp"
#include "Texture.hpp"

struct Material
{
    GLuint _albedo;
    GLuint _normal;
    GLuint _roughness;
    GLuint _metallic;
    GLuint _ao;

    inline static void set_samplers(ShaderProgram& sp, GLenum offset = 0)
    {
        sp.use();
        sp.set_sampler(0 + offset, "s_albedo");
        sp.set_sampler(1 + offset, "s_normal");
        sp.set_sampler(2 + offset, "s_roughness");
        sp.set_sampler(3 + offset, "s_metallic");
        sp.set_sampler(4 + offset, "s_ao");        
    }

    Material(std::string_view dir)
    {
        _albedo = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/albedo.png");
        _normal = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/normal.png");
        _roughness = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/roughness.png");
        _metallic = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/metallic.png");
        _ao = TEXTURE_MANAGER.auto_load_texture(std::string{dir} + "/ao.png");
    }

    void active(GLenum offset = 0) const
    {
        glActiveTexture(GL_TEXTURE0 + offset);
        glBindTexture(GL_TEXTURE_2D, _albedo);

        glActiveTexture(GL_TEXTURE1 + offset);
        glBindTexture(GL_TEXTURE_2D, _normal);

        glActiveTexture(GL_TEXTURE2 + offset);
        glBindTexture(GL_TEXTURE_2D, _roughness);

        glActiveTexture(GL_TEXTURE3 + offset);
        glBindTexture(GL_TEXTURE_2D, _metallic);

        glActiveTexture(GL_TEXTURE4 + offset);
        glBindTexture(GL_TEXTURE_2D, _ao);        
    }

};