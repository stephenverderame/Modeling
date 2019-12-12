#include "Shader.h"
#include <fstream>
#include <sstream>
#include <vector>

void Shader::load(const char * vertexCode, const char * fragCode, const char * geomCode, FILE * msgOut)
{
	unsigned int v, f, g;
	int success;
	char infoLog[512];
	v = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(v, 1, &vertexCode, NULL);
	glCompileShader(v);
	glGetShaderiv(v, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(v, 512, NULL, infoLog);
		fprintf(msgOut, "%s\n", infoLog);
	}
	f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f, 1, &fragCode, NULL);
	glCompileShader(f);
	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(f, 512, NULL, infoLog);
		fprintf(msgOut, "%s\n", infoLog);
	}
	if (geomCode != nullptr) {
		g = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(g, 1, &geomCode, NULL);
		glCompileShader(g);
		glGetShaderiv(g, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(g, 512, NULL, infoLog);
			fprintf(msgOut, "%s\n", infoLog);
		}
	}
	program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	if (geomCode != nullptr) glAttachShader(program, g);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		fprintf(msgOut, "%s\n", infoLog);
	}
	glDeleteShader(v);
	glDeleteShader(f);
	if (geomCode != nullptr) glDeleteShader(g);
}

unsigned int Shader::getProgram(const ShaderPasskey & pass)
{
	return program;
}

Shader::Shader(const ShaderPasskey & key, const char * vertPath, const char * fragPath, const char * geomPath)
{
	std::ifstream in;
	in.open(vertPath);
	std::stringstream sv, sf, sg;
	sv << in.rdbuf();
	in.close();
	in.open(fragPath);
	sf << in.rdbuf();
	in.close();
	if (geomPath != nullptr) {
		in.open(geomPath);
		sg << in.rdbuf();
		in.close();
	}
	load(sv.str().c_str(), sf.str().c_str(), geomPath == nullptr ? nullptr : sg.str().c_str());
}

Shader::~Shader()
{
	glDeleteProgram(program);
}

