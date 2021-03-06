#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tCoords;
layout (location = 2) in vec3 normal;
out vec2 texCoords;
out vec3 fragPos;
out vec3 fragNorm;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	texCoords = tCoords;
	fragPos = vec3(model * vec4(pos, 1.0));
	fragNorm = transpose(inverse(mat3(model))) * normal;
}
