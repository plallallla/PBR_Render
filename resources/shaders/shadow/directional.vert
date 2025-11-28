#version 400 core
layout (location = 0) in vec3 position;


uniform mat4 projection_view;
uniform mat4 model;

void main()
{
    // gl_Position = projection_view * model * vec4(position, 1.0);
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
