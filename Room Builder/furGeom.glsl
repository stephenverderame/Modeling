#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 32) out;

in VS_OUT{
	vec3 normal;
	vec2 texCoords;
} gs_in[];

uniform float time;
const float LENGTH = 0.02;

out vec2 texCoords;
out float light;

vec4 randomPoint(float a, float b)
{
	return (1 - sqrt(a)) * gl_in[0].gl_Position + (sqrt(a) * (1 - b)) * gl_in[1].gl_Position + (b * sqrt(a)) * gl_in[2].gl_Position;
}
int bestInput(vec4 p)
{
	int n = 0;
	for(int i = 1; i < 3; ++i)
	{
		if(distance(gl_in[i].gl_Position.xyz, p.xyz) < distance(gl_in[n].gl_Position.xyz, p.xyz))
			n = i;
	}
	return n;
}
float rand(float x)
{
	return fract(sin(x)*1.0);
}
vec4 pickPoint(float s)
{
	vec4 c = gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position;
	c *= (1.0 / 3.0);
	vec2 x = vec2(gl_in[0].gl_Position.x);
	vec2 y = vec2(gl_in[0].gl_Position.y); 
	vec2 z = vec2(gl_in[0].gl_Position.z);
	for(int i = 1; i < 3; ++i){
		x.x = gl_in[i].gl_Position.x < x.x ? gl_in[i].gl_Position.x : x.x;
		x.y = gl_in[i].gl_Position.x > x.y ? gl_in[i].gl_Position.x : x.y;
		y.x = gl_in[i].gl_Position.y < y.x ? gl_in[i].gl_Position.y : y.x;
		y.y = gl_in[i].gl_Position.y > y.y ? gl_in[i].gl_Position.y : y.y;
		z.x = gl_in[i].gl_Position.z < z.x ? gl_in[i].gl_Position.z : z.x;
		z.y = gl_in[i].gl_Position.z > z.y ? gl_in[i].gl_Position.z : z.y;
	}
	c.x += 0.5 * rand(s) * (x.y - x.x);
	c.y += 0.5 * rand(3 * s * s) * (y.y - y.x);
	c.z += 0.5 * rand(s * s * s) * (z.y - z.x);
	return c;
}
void main()
{
	for(int i = 0; i < 16; ++i)
	{
		vec4 p = pickPoint(0.2 * float(i));
		int index = bestInput(p);
		gl_Position = p;
		texCoords = gs_in[index].texCoords;
		light = 0.2;
		EmitVertex();
		gl_Position = p + vec4(gs_in[index].normal, 0.0) * LENGTH;
		texCoords = gs_in[index].texCoords;
		light = 1.0;
		EmitVertex();		
		EndPrimitive();
	}
/*	for(int i = 0; i < 3; ++i)
	{
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
		gl_Position = gl_in[i].gl_Position + vec4(gs_in[i].normal, 0.0) * LENGTH;
		EmitVertex();
		EndPrimitive();
	}*/
}