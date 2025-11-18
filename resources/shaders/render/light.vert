#version 400 core

out vec2 uv;
out vec3 cube_uv;

uniform mat3 inv_view_matrix3;

void main()
{
    // 全屏三角形的三个顶点（屏幕空间）
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    vec2 pos = positions[gl_VertexID];
    gl_Position = vec4(pos, 0.0, 1.0);

    // 计算 UV（用于 G-buffer 采样）
    uv = pos * 0.5 + 0.5;

    // 构造 View Space 射线方向（指向场景内部）
    // 假设相机朝 -Z，近平面 z = -1（透视投影）
    vec3 view_ray = normalize(vec3(pos, -1.0));

    // 转换到 World Space → 用于 skybox 和背景
    cube_uv = inv_view_matrix3 * view_ray;
}