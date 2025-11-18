# PBR_Render

### 0.why

这是一个基于C++与OpenGL实现的光栅化pbr渲染器，这个项目属于我系统学习图形学光栅化后的第一个综合项目，他可以很好的涵盖之前学习过的大多数知识与技巧，是一次非常好的综合实践。

### 1.what

这个项目主要基于***OpenGL 4.0***进行，使用光栅化实现了***微表面模型***的PBR渲染，流程上主要采用了***延迟渲染管线***实现，支持漫反射辐照度 与镜面预卷积的***IBL***。

渲染器主要有以下特性

- 相机
  - 移动
  - 放缩

- 纹理
  - Cube
  - HDR
  - 2D
- 模型
  - .obj

- 材质
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
- Shader
  - 支持延迟渲染
  - 支持链式离屏渲染
- IBL
  - Diffuse irradiance
  - Specular IBL
- Lighting
  - 方向光源
  - 点光源
  - 传统二次衰弱
  -  *Cook-Torrance* BRDF
- 天空盒
  - Cube贴图

- 阴影
  - shadowmap
  - PCF软阴影
  - 点光源立方体阴影贴图
- 后处理
  - SAO
  - FXAA
  - Bloom
  - TODO:Motion blur
- 其他
  - spdlog
  - IMGUI

### 2.how

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
  - 光源的设计与实现

### 3.reference

[前期预研学习项目](https://github.com/plallallla/CG_Demo)

学习参考自以下内容

[LearnOpenGL](https://learnopengl-cn.github.io)

[pbr-book](https://www.pbr-book.org/3ed-2018/contents)

[Games101](https://www.bilibili.com/video/BV1X7411F744/?spm_id_from=333.788.videopod.episodes&vd_source=35656623bbb678de699bcd2742ccb713)

[Games202](https://www.bilibili.com/video/BV1YK4y1T7yY/?spm_id_from=333.1387.collection.video_card.click&vd_source=35656623bbb678de699bcd2742ccb713)

[***浅墨***的《Real-Time Rendering 3rd》 提炼总结](https://github.com/QianMo/Game-Programmer-Study-Notes/blob/master/Content/《Real-Time%20Rendering%203rd》读书笔记/README.md)

[JoshuaSenouf的gl-engine项目](https://github.com/JoshuaSenouf/gl-engine)