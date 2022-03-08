#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 inColor;

uniform mat4 model;

out vec4 frag_color;

void main()
{
	gl_Position = model * vec4(aPos, 1.0);
	frag_color = inColor;
}