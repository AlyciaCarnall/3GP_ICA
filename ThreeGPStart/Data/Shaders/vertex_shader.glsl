#version 330

uniform mat4 combined_xform;
uniform mat4 model_xform;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 tex_coord;

//render with texture
out vec2 varying_coord;
out vec3 varying_normals;
out vec3 varying_position;

void main(void)
{
	varying_coord = tex_coord;
	varying_normals = mat3(model_xform) * vertex_normal;

	varying_position = mat4x3(model_xform) * vec4(vertex_position, 1.0);

	gl_Position = combined_xform * model_xform * vec4(vertex_position, 1.0);
}