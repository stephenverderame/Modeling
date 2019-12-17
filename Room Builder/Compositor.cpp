#include "Compositor.h"
#include "Composable.h"
#include "ShaderManager.h"
#include <vector>
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shader.h"

const float rectVerts[] = 
{
	0.0, 1.0,
	0.0, 0.0,
	1.0, 1.0,
	1.0, 0.0
};
struct cmImpl
{
	std::vector<std::reference_wrapper<Composable>> inputs;
	unsigned int vao, vbo;
	unsigned int w, h;
};

Compositor::Compositor(unsigned int width, unsigned int height)
{
	pimpl = std::make_unique<cmImpl>();
	pimpl->w = width;
	pimpl->h = height;
	glGenVertexArrays(1, &pimpl->vao);
	glGenBuffers(1, &pimpl->vbo);
	glBindVertexArray(pimpl->vao);
	glBindBuffer(GL_ARRAY_BUFFER, pimpl->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectVerts), rectVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glm::mat4 proj = glm::ortho(0.f, (float)width, 0.f, (float)height, -1.f, 1.f);
	ShaderManager::getShader(shaderID::compositor)->setInt("samples", 16);
	ShaderManager::getShader(shaderID::compositor)->setMat4("projection", proj);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0));
	model = glm::scale(model, glm::vec3(width, height, 1.0));
	ShaderManager::getShader(shaderID::compositor)->setMat4("model", model);
	ShaderManager::getShader(shaderID::gui)->setMat4("projection", glm::ortho(0.f, (float)width, 0.f, (float)height, -1.f, 1.f));
}


Compositor::~Compositor() = default;

void Compositor::compose(Composable & comp)
{
	pimpl->inputs.push_back(comp);
}

void Compositor::renderFinal()
{

	ShaderManager::useShader(shaderID::compositor);
	glBindVertexArray(pimpl->vao);
	glDisable(GL_DEPTH_TEST);
	for (auto in : pimpl->inputs)
	{
		in.get().compose();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}
	glEnable(GL_DEPTH_TEST);
}

void Compositor::notify(const message & msg)
{
	if (msg.msg == WM_SIZE) {
		ShaderManager::getShader(shaderID::compositor)->setMat4("projection", glm::ortho(0.f, (float)LOWORD(msg.lparam), 0.f, (float)HIWORD(msg.lparam), -1.f, 1.f));
		ShaderManager::getShader(shaderID::compositor)->setMat4("model", glm::scale(glm::mat4(), glm::vec3(LOWORD(msg.lparam), HIWORD(msg.lparam), 1.0)));
		ShaderManager::getShader(shaderID::gui)->setMat4("projection", glm::ortho(0.f, (float)LOWORD(msg.lparam), 0.f, (float)HIWORD(msg.lparam), -1.f, 1.f));
	}
}
