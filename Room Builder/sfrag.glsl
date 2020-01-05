#version 330 core
in vec2 texCoords;
out vec4 FragColor;
in vec3 fragPos;
in vec3 fragNorm;
uniform vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
uniform sampler2D diffuseTex;
uniform samplerCube reflectTex;
uniform samplerCube shadowTex;
uniform int useTex;
uniform vec3 viewPos;

uniform vec4 clipPlane;
uniform bool clip;

uniform vec3 objNormal;
uniform vec3 camPos;

uniform vec3 lightPos;
uniform vec3 lightColor = vec3(0.3);
uniform float farPlane;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
); 

float calcShadow(vec3 pos)
{
	vec3 fragToLight = pos - lightPos;
	float viewDistance = length(viewPos - pos);
	int samples = 20;
	float bias = 0.15;
	float sampleRadius = (1.0 + (viewDistance / farPlane)) / 25.0;
	float shadow = 0.0;
	float currentDepth = length(fragToLight);
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(shadowTex, fragToLight + sampleOffsetDirections[i] * sampleRadius).r;
		closestDepth *= farPlane;
		shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
	}
	return shadow / float(samples);

}
void main()
{
//	if(clip && dot(fragPos, clipPlane.xyz) < clipPlane.w) discard;

	if(useTex < 2)
	{
		vec4 color = useTex == 1 ? texture(diffuseTex, texCoords) : color;
		vec3 normal = normalize(fragNorm);
		vec3 ambient = 0.3 * color.rgb;
		vec3 lightDir = normalize(lightPos - fragPos);
		float diff = max(dot(lightDir, normal), 0.0);
		vec3 diffuse = diff * lightColor;
		vec3 viewDir = normalize(camPos - fragPos);
		vec3 reflectDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, reflectDir), 0.0), 64.0);
		vec3 specular = spec * lightColor;
		float shadow = calcShadow(fragPos);
		FragColor.rgb = (ambient + (1.0 - shadow) * (diffuse + specular)) * vec3(color);
		FragColor.a = color.a;

	}
	else if(useTex == 2)
	{
		vec3 viewDir = normalize(fragPos - camPos);
		vec3 r = reflect(viewDir, normalize(fragNorm));
		FragColor = texture(reflectTex, r);

//		FragColor = texture(reflectTex, texCoords);
		FragColor.a = 1.0;
	}
}
