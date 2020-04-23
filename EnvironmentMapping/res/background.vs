#version 450 core

layout (location = 0) in vec3 a_Position;

uniform mat4 u_projection;
uniform mat4 u_view;

out vec3 v_worldPos;

void main()
{
    v_worldPos = a_Position;

	mat4 rotView = mat4(mat3(u_view));
	vec4 clipPos = u_projection * rotView * vec4(v_worldPos, 1.0);

	gl_Position = clipPos.xyww;
}