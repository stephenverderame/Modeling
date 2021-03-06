#pragma once
#include <memory>
#include "Observer.h"
#include <glm.hpp>
struct camImpl;
class Camera : public Observer
{
	std::unique_ptr<camImpl> pimpl;
private:
	void updateCam();
	struct vec4 screenToWorld(struct vec2 mouse);
public:
	Camera();
	~Camera();
	void setViewMatrix(enum class shaderID sid);
	void pan(float dx, float dy);
	void zoom(float dz);
	void rotate(float dx, float dy);
	void notify(const command & msg) override;
	bool isInterested(msg m) override;
	void attach(Observer & ob);
	glm::vec3 getPos();
	glm::mat4 getViewMatrix();
};

