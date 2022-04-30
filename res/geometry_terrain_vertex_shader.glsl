#version 430 core

layout(location = 0) in vec2 vert;
layout(location = 0) uniform mat4 mvp;
layout(location = 4) uniform float min_height;
layout(location = 5) uniform float max_height;
uniform sampler2D heightmap;

out vec3 pos;

void main()
{
    vec2 uv = (vert + 1.)/2.;

    vec4 real_vert = vec4(vert.x, texture(heightmap, uv).r * (max_height - min_height) + min_height, vert.y ,1);

    gl_Position = mvp * real_vert;
}

