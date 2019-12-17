#version 330 core
in vec2 texCoords;
uniform sampler2DMS texMS;
uniform int samples = 4;

out vec4 fragColor;
vec4 sampleMS()
{
	ivec2 tSize = textureSize(texMS);
	ivec2 tCoords = ivec2(tSize * texCoords);
	vec4 color = vec4(0);
	for(int i = 0; i < samples; ++i){
		color += texelFetch(texMS, tCoords, i);
	}
	color /= float(samples);
	return color;
}
void main()
{
	fragColor = sampleMS();
}