#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out; //take in triangles (3 verts) and output one triangle on each of the 6 layers (18 verts)

uniform mat4 lightSpaceTransforms[6]; //each matrix for each different face of the cubemap (each direction)

out vec4 fragPos;

void main()
{
	for(int i = 0; i < 6; ++i)
	{
		gl_Layer = i; //updates which face of the cubemap to output to
		for(int j = 0; j < 3; ++j)
		{
			fragPos = lightSpaceTransforms[i] * gl_in[j].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}