#version 430 core

in vec2 uv;
layout(binding = 0, location = 1) uniform sampler2D text;

out vec3 color;

void main()
{
	color = texture(text, uv).rgb;
}
