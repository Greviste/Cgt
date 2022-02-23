#version 430 core

layout(location = 0) in vec4 vert;
layout(location = 2) in vec2 vert_uv;
layout(location = 0) uniform mat4 mvp;

out vec2 uv;

void main()
{
    uv = vert_uv;
    gl_Position = mvp*vert;
}

