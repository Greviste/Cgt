#version 330 core

in vec2 uv;
uniform sampler2D heightmap;
uniform sampler2D grass_texture;
uniform sampler2D rock_texture;
uniform sampler2D snow_texture;

out vec3 color;

vec4 trimix(vec4 v1, vec4 v2, vec4 v3, float a, float start_mix, float end_mix, float start_mix2, float end_mix2)
{
	return mix(v1, mix(v2, v3, clamp((a-start_mix2)/end_mix2, 0, 1)), clamp((a-start_mix)/end_mix, 0, 1));
}

void main()
{
	float height = texture(heightmap, uv).r;
	color = trimix(texture(grass_texture, uv), texture(rock_texture, uv), texture(snow_texture, uv), height, 0.2, 0.4, 0.5, 0.55).rgb;
}
