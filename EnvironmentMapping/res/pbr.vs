#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;


uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;


out vec3 v_Normal;
out vec3 v_WorldPosition;
out vec2 v_TexCoords;


void main()
{
	v_WorldPosition = vec3(u_model * vec4(a_Position, 1.0));
	v_TexCoords = a_TexCoord;
	v_Normal =  normalize(transpose(inverse(mat3(u_model))) * a_Normal);
    gl_Position = u_projection * u_view  * vec4(v_WorldPosition, 1.0);
}