#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
uniform mat4 mvp;
uniform sampler2D heightmap;
out vec2 uv;

void main()
{
    vec4 pos = vec4(vertices_position_modelspace,1);
    uv = (vertices_position_modelspace.xz + 1.)/2.;
    pos.y += texture(heightmap, uv).r;
    gl_Position = mvp*pos;
}

