#version 430 core

layout(location = 0) in vec2 pos;
layout(location = 0) uniform mat4 mvp;
layout(location = 1) uniform float min_height;
layout(location = 2) uniform float max_height;
uniform sampler2D heightmap;

out vec2 uv;

void main()
{
    uv = (pos + 1.)/2.;
    vec4 pos = vec4(pos.x, texture(heightmap, uv).r * (max_height - min_height) + min_height, pos.y ,1);
    gl_Position = mvp*pos;
}

