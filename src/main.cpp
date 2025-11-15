#include "Camera.hpp"
#include "GLWidget.hpp"
#include "QuadRender.hpp"
#include "Precompution.hpp"
#include "SkyboxRender.hpp"
#include "Material.hpp"
#include "Model.hpp"

#include "PostRender.hpp"
#include "Texture.hpp"
#include "utility.hpp"



class PBR_render : public GLWidget
{

    Material rusted_iron{TEXTURE_PATH + "pbr/grass"};

    // 天空盒渲染pass
    GLuint _input_hdr = TEXTURE_MANAGER.load_hdr_texture(TEXTURE_PATH + "hdr/newport_loft.hdr");
    SkyboxRender _skybox;

    // 预处理渲染
    BRDF_LUT budf_lut;
    EquirectConvertRender equirect_pass;
    ConvolutionIBLRender convolution_pass;
    PrefilterIBLRender prefilter_pass;

    PostRender _debug;

    virtual void application() override
    {
        budf_lut.execute();
        equirect_pass.execute(_input_hdr);
        convolution_pass.execute(equirect_pass);
        prefilter_pass.execute(equirect_pass);
    }

    virtual void render_loop() override
    {
        update_viewport();
        // _skybox.render_texture(equirect_pass, get_projection());
        // _skybox.render_texture(prefilter_pass, get_projection());
        // _skybox.render_texture(prefilter, get_projection());
        // _debug.render_texture(budf_lut);
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