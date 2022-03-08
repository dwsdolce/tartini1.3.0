#version 330 core

uniform vec4 frag_color;

out vec4 FragColor;

void main()
{
	FragColor = frag_color;
}