#include "Object.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <glad\glad.h>
#include "ShaderManager.h"
#include "Shader.h"
#include <vector>
#include "Font.h"

const float cubeVerts[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
const float outlineVerts[] = {
	0.0, 0.0, 1.0,
	1.0, 0.0, 1.0,
	1.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
};
const float rectVerts[] = {
	0.0, 1.0, 0.0,
	0.0, 0.0, 0.0,
	1.0, 1.0, 0.0,
	1.0, 0.0, 0.0
};
struct oImpl
{
	std::vector<std::reference_wrapper<ObjectDecorator>> decorators;
	bool cancelDraw;
};
Object::Object() : selected(false)
{
	pimpl = std::make_unique<oImpl>();
	pimpl->cancelDraw = false;
}

Object::~Object() = default;

void Object::draw()
{
	ShaderManager::useShader(sid);
	nvi_draw();
}

void Object::select(bool s)
{
	selected = s;
}
glm::vec3 Object::getPos()
{
	glm::vec3 p;
	p.x = model[3][0];
	p.y = model[3][1];
	p.z = model[3][2];
	return p;
}

void Object::translate(glm::vec3 & pos)
{
	model = glm::translate(model, pos);
}
void Object::setPos(glm::vec3 & pos)
{
	model[3][0] = pos.x;
	model[3][1] = pos.y;
	model[3][2] = pos.z;
}
void Object::rotate(glm::vec3 & axis, float angle)
{
	model = glm::rotate(model, angle, axis);
}
void Object::scale(glm::vec3 & scale)
{
	model = glm::scale(model, scale);
}
void Object::transform(glm::mat4 & mat)
{
	model = model * mat;
}
void Object::translate(glm::vec3 && pos)
{
	model = glm::translate(model, pos);
}
void Object::setPos(glm::vec3 && pos)
{
	model[3][0] = pos.x;
	model[3][1] = pos.y;
	model[3][2] = pos.z;
}
void Object::rotate(glm::vec3 && axis, float angle)
{
	model = glm::rotate(model, angle, axis);
}
void Object::scale(glm::vec3 && scale)
{
	model = glm::scale(model, scale);
}
void Object::transform(glm::mat4 && mat)
{
	model = model * mat;
}
void Object::clearMatrix()
{
	model = glm::mat4();
}
void Object::addDecorator(ObjectDecorator & decorator)
{
	if (decorator.overrideDraw()) pimpl->cancelDraw = true;
	pimpl->decorators.emplace_back(decorator);
	glBindVertexArray(vao);
	decorator.init();
}
Cube::Cube() 
{
	sid = shaderID::basic;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
}
Cube::~Cube()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}
void Cube::nvi_draw()
{
	glBindVertexArray(vao);
	if (!pimpl->cancelDraw) {
		if (selected) {
			ShaderManager::getShader(sid)->setMat4("model", glm::scale(model, glm::vec3(1.01)));
			ShaderManager::getShader(sid)->setVec4("color", glm::vec4(0, 0, 1, 1));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		ShaderManager::getShader(sid)->setMat4("model", model);
		ShaderManager::getShader(sid)->setVec4("color", glm::vec4(1, 0, 0, 1));
		if(selected) glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		if(selected) glEnable(GL_DEPTH_TEST);
	}
	for (auto& d : pimpl->decorators) {
		shaderID custom;
		if (d.get().useCustomShader(custom))
			ShaderManager::getShader(custom)->setMat4("model", model);
		d.get().decorate(GL_TRIANGLES, 0, 36);
	}
}

Rect::Rect(bool outline) : outline(outline)
{
	sid = shaderID::basic;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if (outline)
		glBufferData(GL_ARRAY_BUFFER, sizeof(outlineVerts), outlineVerts, GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, sizeof(rectVerts), rectVerts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
}
Rect::~Rect()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}
void Rect::nvi_draw()
{
	ShaderManager::getShader(sid)->setMat4("model", model);
	glBindVertexArray(vao);
	int mode = outline ? GL_LINE_LOOP : GL_TRIANGLE_STRIP;
	if (!pimpl->cancelDraw) {
		glDrawArrays(mode, 0, 4);
	}
	for (auto& d : pimpl->decorators) {
		shaderID custom;
		if (d.get().useCustomShader(custom))
			ShaderManager::getShader(custom)->setMat4("model", model);
		d.get().decorate(mode, 0, 4);
	}
}
struct comImpl 
{
	std::vector<std::reference_wrapper<Object>> objects;
};
CompositeObject::CompositeObject()
{
	pimpl = std::make_unique<comImpl>();
}

CompositeObject::~CompositeObject() = default;

void CompositeObject::nvi_draw()
{
	for (auto& o : pimpl->objects) {
		o.get().transform(model);
		o.get().draw();
	}
}

void CompositeObject::addObject(Object & obj)
{
	pimpl->objects.emplace_back(obj);
}

struct flyImpl
{
	std::vector<glm::mat4> models;
	Object * obj;
};
FlyweightObject::FlyweightObject(int count)
{
	pimpl = std::make_unique<flyImpl>();
	pimpl->models.resize(count);
}

FlyweightObject::~FlyweightObject() = default;

void FlyweightObject::nvi_draw()
{
	for (auto& mat : pimpl->models) {
		pimpl->obj->clearMatrix();
		pimpl->obj->transform(mat);
		pimpl->obj->draw();
	}
}

void FlyweightObject::setObject(Object & obj)
{
	pimpl->obj = &obj;
}

void FlyweightObject::transformObj(int index, glm::mat4 & transform)
{
	pimpl->models[index] = transform;
}

void FlyweightObject::setCount(int newCount)
{
	pimpl->models.resize(newCount);
}

int FlyweightObject::getCount()
{
	return pimpl->models.size();
}

InstancedObject::InstancedObject(glm::vec3 * offsets, int offsetNum) : instances(offsetNum), offsets(offsets)
{
	sid = shaderID::instance;
	glGenBuffers(1, &instancedVBO);

}

InstancedObject::~InstancedObject()
{
	glDeleteBuffers(1, &instancedVBO);
}

void InstancedObject::decorate(int mode, int start, int count)
{
	glDrawArraysInstanced(mode, start, count, instances);
}

void InstancedObject::setOffset(glm::vec3 offset, int offsetIndex)
{
	glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
	glBufferSubData(GL_ARRAY_BUFFER, offsetIndex * sizeof(glm::vec3), sizeof(glm::vec3), &offset);
}

void InstancedObject::init()
{
	glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
	glBufferData(GL_ARRAY_BUFFER, instances * sizeof(glm::vec3), offsets, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glVertexAttribDivisor(1, 1); //attribute 1 updates every instance
}

bool InstancedObject::useCustomShader(shaderID &newShader)
{
	newShader = shaderID::instance;
	return true;
}

struct txImpl
{
	std::shared_ptr<Font> fnt;
	const char * txt;
	float scale;
	glm::vec4 color;
};

Text::Text(std::shared_ptr<class Font> fnt)
{
	pimpl = std::make_unique<txImpl>();
	pimpl->fnt = fnt;
	sid = shaderID::gui;
}
Text::~Text() = default;

void Text::nvi_draw()
{
	glm::vec3 v = getPos();
	ShaderManager::getShader(shaderID::gui)->setVec4("color", pimpl->color);
	pimpl->fnt->drawText(pimpl->txt, v.x, v.y, pimpl->scale);
}

void Text::setText(const char * txt, float scale)
{
	pimpl->txt = txt;
	pimpl->scale = scale;
}
void Text::setColor(glm::vec4 color) 
{
	pimpl->color = color;
}
