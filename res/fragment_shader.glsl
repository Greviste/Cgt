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
in vec3 normal;
in vec2 uv;

layout(binding = 0) uniform sampler2D text;
layout(location = 9) uniform float shininess = 4.;
layout(location = 10) uniform Light lights[n_lights]; //slots [10, 106[

out vec3 color;

void main()
{
    vec3 n = normalize(normal);

    vec3 wo = normalize(-pos); // unit vector pointing to the camera
    vec3 irradiance = vec3(0, 0, 0);

    for (int i = 0; i < n_lights; ++i)
    {
        if (lights[i].isActive == 1)
        {
            vec4 shadowPos = lights[i].vp * vec4(pos, 1);
            vec3 shadowCoords = (shadowPos.xyz / shadowPos.w + 1.) * 0.5;

            float influence = shadowCoords.x < 0 || shadowCoords.x > 1 || shadowCoords.y < 0 || shadowCoords.y > 1 ? 0. :
                (texture(lights[i].shadowMap, shadowCoords.xy).x + 0.001 >= shadowCoords.z ? 1. : 0.);

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
                    * (max(dot(n, wi), 0.0) + pow(max(dot(n, wh), 0.0), shininess)) / dist2
                    ;
            }
        }
    }

	color = (irradiance + 0.1) * texture(text, uv).rgb;
}
