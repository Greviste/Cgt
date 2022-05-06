#version 430 core

struct Light
{
    vec3 pos; //+0
    vec3 color; //+1
    float intensity; //+2
    mat4 vp; //+3
    sampler2D shadowMap; //+4
    int isActive; //+5
};//6 uniform slots
const int n_lights = 16;

in vec3 pos;
in vec2 uv;

uniform mat4 nmat;
layout(binding = 0) uniform sampler2D heightmap;
layout(binding = 1) uniform sampler2D grass_texture;
layout(binding = 2) uniform sampler2D rock_texture;
layout(binding = 3) uniform sampler2D snow_texture;
layout(location = 10) uniform Light lights[n_lights]; //slots [10, 106[

out vec3 color;

vec4 trimix(vec4 v1, vec4 v2, vec4 v3, float a, float start_mix, float end_mix, float start_mix2, float end_mix2)
{
	return mix(v1, mix(v2, v3, clamp((a-start_mix2)/end_mix2, 0, 1)), clamp((a-start_mix)/end_mix, 0, 1));
}

void main()
{
    float step = 1. / float(textureSize(heightmap, 0).x);
    vec2 derivatives = vec2(texture(heightmap, uv + vec2(step, 0)).r - texture(heightmap, uv - vec2(step, 0)).r,
        texture(heightmap, uv + vec2(0, step)).r - texture(heightmap, uv - vec2(0, step)).r);
    vec3 n = normalize((nmat * vec4(-derivatives.x, 2 * step, -derivatives.y, 1)).xyz);

    vec3 tangent = n.y == 1.f ? vec3(1, 0, 0) : vec3(0, 1, 0);
    vec3 bitangent = normalize(cross(n, tangent));
    tangent = cross(bitangent, n);

    vec3 wo = normalize(-pos); // unit vector pointing to the camera
    vec3 irradiance = vec3(0, 0, 0);

    for (int i = 0; i < n_lights; ++i)
    {
        if (lights[i].isActive == 1)
        {
            float influence = 0;
            float influencePerSample = 1.f / 16;
            for (int xi = 0; xi < 4; ++xi)
            {
                for (int yi = 0; yi < 4; ++yi)
                {
                    vec3 offsetPos = pos + ((xi - 1.5) * tangent + (yi - 1.5) * bitangent) * 0.01;
                    vec4 shadowPos = lights[i].vp * vec4(offsetPos, 1);
                    vec3 shadowCoords = (shadowPos.xyz / shadowPos.w + 1.) * 0.5;

                    influence += shadowCoords.x < 0 || shadowCoords.x > 1 || shadowCoords.y < 0 || shadowCoords.y > 1 ? 0. :
                        (texture(lights[i].shadowMap, shadowCoords.xy).x > shadowCoords.z ? influencePerSample : 0.);
                }
            }

            vec3 toLight = lights[i].pos - pos;
            float dist2 = dot(toLight, toLight);
            vec3 wi = normalize(toLight); // unit vector pointing to the light source
            if (dot(wi, n) >= 0.0) // WE ONLY CONSIDER LIGHTS THAT ARE ON THE RIGHT HEMISPHERE (side of the tangent plane)
            {
                vec3 wh = normalize(wi + wo); // half vector (if wi changes, wo should change as well)
                vec3 Li = lights[i].color * lights[i].intensity;

                irradiance = irradiance +
                    Li // light color
                    * influence
                    * max(dot(n, wi), 0.0) / dist2
                    ;
            }
        }
    }

	float height = texture(heightmap, uv).r;
    color = (irradiance + 0.1) * trimix(texture(grass_texture, uv), texture(rock_texture, uv), texture(snow_texture, uv), height, 0.2, 0.4, 0.5, 0.55).rgb;
}
