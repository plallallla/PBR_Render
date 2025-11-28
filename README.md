# PBR_Render

### 0. why

这是一个基于C++与OpenGL实现的光栅化的PBR渲染器，基本涵盖了经典教程[LearnOpenGL](https://learnopengl-cn.github.io)中的绝大多数内容，如果你学习完基本的一些图形学知识与教学内容，可以参考这个作为一次综合实践。

### 1. what

这个项目主要基于***OpenGL 4.0***进行，使用光栅化实现了***微表面模型***的PBR渲染，流程上主要采用了***延迟渲染管线***实现，支持漫反射辐照度 与镜面预卷积的***IBL***。

渲染器主要有以下特性

- Camera
  - Move
  - Zoom
  - TODO:Exposure
    - Aperture
    - Shutter speed
    - ISO
- Texture
  - Cube
  - HDR
  - 2D
- 模型
  - .obj
- Material
  - Cook-Torrance BRDF
    - Albedo
    - Normal
    - Roughness
    - Metalness
    - AO
  - Phong
    - ambient
    - diffuse
    - specular
    - shininess
- 渲染流程
  - Deffered
  - Instance
  - TODO:forward for transparent
- IBL
  - Diffuse irradiance
  - Specular IBL
- Lighting
  - Directional
  - Point
  - TODO:Spot
  - TODO:Volume Light
  -  *Cook-Torrance* BRDF
- SkyBox
  - Cube
- Shadow
  - 包围球模拟的方向光源
  - 点光源立方体阴影贴图
  - 软阴影
    - PCF
    - TODO:PCSS
- Post Process
  - TODO:SAO
  - Anti-aliasing
    - FXAA
    - TODO:TAA
  - TODO:Bloom
  - Motion blur
- 其他
  - spdlog
  - IMGUI

### 2. how

$$
L_o \approx
\underbrace{
\sum_{j} \left[
(1 - F_j)(1 - m) \frac{\text{albedo}}{\pi} + \frac{D_j G_j F_j}{4 (\mathbf{n}\cdot\mathbf{v})(\mathbf{n}\cdot\mathbf{l}_j)}
\right] L_j (\mathbf{n} \cdot \mathbf{l}_j)
}_{\text{Direct Lighting}}
+
\underbrace{
(1 - F_{\text{env}})(1 - m) \cdot \text{Irradiance}(\mathbf{n}) \cdot \text{albedo}
+ 
\text{Prefilter}(\mathbf{r}, \text{rough}) \cdot \left( F_{\text{env}} \cdot \text{BRDF}_x + \text{BRDF}_y \right)
}_{\text{Indirect Lighting (IBL)}}
\cdot \text{AO}
$$

TODO：补充渲染流程

TODO：补充实现博客

- 几何阶段
  - 空间坐标系与变换矩阵
  - 法线变换矩阵
  - 切线与副切线
- 着色阶段
  - 微表面模型brdf
  - IBL
  - 光源与阴影

### 3. reference

[练手仓库链接](https://github.com/plallallla/CG_Demo)

学习参考自以下内容：

[LearnOpenGL](https://learnopengl-cn.github.io)

[pbr-book](https://www.pbr-book.org/3ed-2018/contents)

[Games101](https://www.bilibili.com/video/BV1X7411F744/?spm_id_from=333.788.videopod.episodes&vd_source=35656623bbb678de699bcd2742ccb713)

[Games202](https://www.bilibili.com/video/BV1YK4y1T7yY/?spm_id_from=333.1387.collection.video_card.click&vd_source=35656623bbb678de699bcd2742ccb713)

[***浅墨***的《Real-Time Rendering 3rd》 提炼总结](https://github.com/QianMo/Game-Programmer-Study-Notes/blob/master/Content/《Real-Time%20Rendering%203rd》读书笔记/README.md)

[JoshuaSenouf的gl-engine项目](https://github.com/JoshuaSenouf/gl-engine)