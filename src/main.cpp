#include "Camera.hpp"
#include "GLWidget.hpp"
#include <OpenGL/gltypes.h>
#include "QuadRender.hpp"

#include "Precompution.hpp"
#include "SkyboxRender.hpp"

#include "Material.hpp"

class PBR_render : public GLWidget
{
    SkyboxRender _skybox;
    EquirectConvertRender hdr_pass;
    DiffuseIrradianceIBL di_pass;
    BRDF_LUT budf_lut;
    SpecularPrefilterIBL prefilter;
    SkyboxRender _sky;
    GLuint hdrTexture;
    Material rusted_iron{TEXTURE_PATH + "pbr/gold"};

    virtual void application() override
    {

    }

    virtual void render_loop() override
    {

    }

    public:
    PBR_render(int width, int height, std::string_view title) : GLWidget(width,height,title) 
    {
    }
};


int main()
{
    PBR_render pbr_render_widget{900, 800, "pbr_render"};
    pbr_render_widget.render();
    return 0;
}