#version 430 core

layout(location = 0) in vec4 vert;
layout(location = 0) uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vert;
}

