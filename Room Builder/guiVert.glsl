#version 330 core
layout(location = 0) in vec2 pos;
uniform mat4 model;
uniform mat4 projection;
uniform ivec2 screenSize;
uniform bool text;
out vec2 texCoords;
flat out int isText;
void main()
{
	mat4 m = model;
//	if(!text){
		m[3][0] *= float(screenSize.x);
		m[3][1] *= float(screenSize.y);
		m[0][0] *= float(screenSize.x);
		m[1][1] *= float(screenSize.y);
//	}
	isText = text ? 1 : 0;
	gl_Position = projection * m * vec4(pos, 0.0, 1.0);
	texCoords = pos;
}