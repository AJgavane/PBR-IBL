#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;


uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;


out vec3 v_worldPos;

void main()
{
	v_worldPos = a_Position;
    gl_Position = u_projection * u_view  * vec4(v_worldPos, 1.0);
}