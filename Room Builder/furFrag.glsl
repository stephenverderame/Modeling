#version 330 core
out vec4 fragColor;
in vec2 texCoords;
in float light;
uniform sampler2D diffuseTex;
void main()
{
	fragColor = texture(diffuseTex, texCoords);
	fragColor.rgb *= light;
}