#version 430 core

layout(location = 0) in vec4 vert;
layout(location = 1) in vec4 vert_normal;
layout(location = 2) in vec2 vert_uv;
layout(location = 0) uniform mat4 m;
layout(location = 1) uniform mat4 v;
layout(location = 2) uniform mat4 p;
layout(location = 3) uniform mat4 nmat;

out vec3 pos;
out vec3 normal;
out vec2 uv;

void main()
{
    vec4 pos4 = v * m * vert;
    pos = pos4.xyz;

    normal = normalize((nmat * vert_normal).xyz);

    uv = vert_uv;

    gl_Position = p * pos4;
}

