#version 330 core
in vec4 fragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
	float lightDistance = length(fragPos.xyz - lightPos);
	gl_FragDepth = lightDistance / farPlane; //map to 0 to 1 range
}