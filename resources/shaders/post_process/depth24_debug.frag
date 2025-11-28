#version 330 core
out vec4 FragColor;

in vec2 uv;

uniform sampler2D screenTexture;

const float near_plane = 0.1;
const float far_plane = 75.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{             
    float depthValue = texture(screenTexture, uv).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}