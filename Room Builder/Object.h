#pragma once
#include <memory>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
class ObjectDecorator {
public:
	virtual void decorate(int drawType, int drawStart, int drawCount) = 0;
	virtual bool overrideDraw() = 0;
	virtual bool useCustomShader(enum class shaderID & newShader) = 0;
	virtual void init() {};
};
struct oImpl;
class Object
{
protected:
	unsigned int vao;
	enum class shaderID sid;
	glm::mat4 model;
	bool selected;
	std::unique_ptr<oImpl> pimpl;
protected:
	virtual void nvi_draw() = 0;
public:
	Object();
	virtual ~Object();
	void draw();
	void translate(glm::vec3 & pos);
	void setPos(glm::vec3 & pos);
	void rotate(glm::vec3 & axis, float angle);
	void scale(glm::vec3 & scale);
	void transform(glm::mat4 & mat);
	void translate(glm::vec3 && pos);
	void setPos(glm::vec3 && pos);
	void rotate(glm::vec3 && axis, float angle);
	void scale(glm::vec3 && scale);
	void transform(glm::mat4 && mat);
	void clearMatrix();
	void addDecorator(ObjectDecorator & decorator);

	void select(bool s);
	glm::vec3 getPos();
};
class Cube : public Object
{
	unsigned int vbo;
public:
	Cube();
	~Cube();
protected:
	void nvi_draw() override;
};
class Rect : public Object
{
	unsigned int vbo;
	bool outline;
public:
	Rect(bool outlineOnly = false);
	~Rect();
	void setOutlineMode(bool outline) { this->outline = outline; }
protected:
	void nvi_draw() override;
};
struct comImpl;
class CompositeObject : public Object {
	std::unique_ptr<comImpl> pimpl;
public:
	CompositeObject();
	~CompositeObject();
	virtual void addObject(Object & obj);
protected:
	virtual void nvi_draw() override;
};
struct flyImpl;
class FlyweightObject : public Object {
	std::unique_ptr<flyImpl> pimpl;
public:
	FlyweightObject(int count = 10);
	~FlyweightObject();
	virtual void setObject(Object & obj);
	virtual void transformObj(int index, glm::mat4 & transform);
	void setCount(int newCount);
	int getCount();
protected:
	virtual void nvi_draw() override;
};
class InstancedObject : public ObjectDecorator
{
	unsigned int instancedVBO;
	int instances;
	enum class shaderID sid;
	glm::vec3 * offsets;
public:
	InstancedObject(glm::vec3 * offsets, int offsetNum);
	~InstancedObject();
	bool overrideDraw() override { return true; };
	void decorate(int mode, int start, int count) override;
	bool useCustomShader(enum class shaderID & newShader) override;
	void setOffset(glm::vec3 offset, int offsetIndex);
	void init() override;
};
struct txImpl;
class Text : public Object
{
	std::unique_ptr<txImpl> pimpl;
public:
	Text(std::shared_ptr<class Font> fnt);
	void nvi_draw() override;
	void setText(const char * txt, float scale = 1.0);
	void setColor(glm::vec4 color);
	~Text();
};


