#version 330 core
in vec2 texCoords;
flat in int isText;
uniform sampler2D tex;
uniform ivec4 glyphData;
uniform ivec2 texData;
uniform vec4 color;

out vec4 fragColor;

const float smoothing = 1.0/16.0;
void main()
{
	if(isText == 1){
		vec2 coords;
		coords.x = float(glyphData.x) + texCoords.x * float(glyphData.z);
		coords.y = float(glyphData.y) + float(glyphData.w) - (texCoords.y * float(glyphData.w));
		coords.x /= float(texData.x);
		coords.y /= float(texData.y);
		float dist = texture2D(tex, coords).a;
		float a = smoothstep(0.5 - smoothing, 0.5 + smoothing, dist);
		fragColor = vec4(color.rgb, color.a * a);
	}
	else
		fragColor = color;
}