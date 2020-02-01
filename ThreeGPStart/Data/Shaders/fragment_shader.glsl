#version 330

uniform sampler2D sampler_tex;

in vec2 varying_coord;
in vec3 varying_normals;
in vec3 varying_position;

out vec4 fragment_colour;

void main(void)
{
	vec3 light_position = vec3(0, 400, 0);

	//render with texture
	vec3 tex_colour = texture(sampler_tex, varying_coord).rgb;

	vec3 P = varying_position;

	vec3 light_direction = P - light_position;

	vec3 L = normalize(-light_direction);
	vec3 N = normalize(varying_normals);

	float diffuse_intensity = max(0, dot(L,N));

	vec3 light_colour = vec3(1,0.2,0.2);

	vec3 final_colour = tex_colour + diffuse_intensity + light_colour;

	fragment_colour = vec4(tex_colour, 1.0);

}