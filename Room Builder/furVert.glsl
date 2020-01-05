#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tCoords;
layout (location = 2) in vec3 normal;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out VS_OUT{
	vec3 normal;
	vec2 texCoords;
} vs_out;

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	vs_out.texCoords = tCoords;
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	vs_out.normal = normalize(vec3(projection * vec4(normalMatrix * normal, 0.0)));
}
