#version 330 core
in vec2 texCoords;
out vec4 FragColor;
uniform vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
uniform sampler2D diffuseTex;
uniform sampler2D reflectTex;
uniform int useTex;
void main()
{
	if(useTex == 1)
		FragColor = texture(diffuseTex, texCoords);
	else if(useTex == 2){
		FragColor = texture(reflectTex, texCoords);
		FragColor.a = 1.0;
	}else
		FragColor = color;
}
