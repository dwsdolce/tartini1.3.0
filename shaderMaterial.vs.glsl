#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()	
{
    // Position in world coordinates
    FragPos = vec3(model * vec4(aPos, 1.0));
	// To handle non-uniform scaling use the Normal matrix instead of just the model transform of the
	// normal. Then use it to transform the normal to world coordinates.
	Normal = mat3(transpose(inverse(model))) * aNormal;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}

