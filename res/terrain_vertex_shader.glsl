#version 430 core

layout(location = 0) in vec2 vert;
layout(location = 0) uniform mat4 m;
layout(location = 1) uniform mat4 v;
layout(location = 2) uniform mat4 p;
layout(location = 3) uniform mat4 nmat;
layout(location = 4) uniform float min_height;
layout(location = 5) uniform float max_height;
uniform sampler2D heightmap;

out vec3 pos;
out vec2 uv;


void main()
{
    uv = (vert + 1.)/2.;

    vec4 real_vert = vec4(vert.x, texture(heightmap, uv).r * (max_height - min_height) + min_height, vert.y ,1);
    vec4 pos4 = v * m * real_vert;
    pos = pos4.xyz;

    gl_Position = p * pos4;
}

