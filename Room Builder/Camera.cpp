#include "Camera.h"
#include "ShaderManager.h"
#include "Shader.h"
#include <glm.hpp>
#include "Window.h"
#include <gtc/matrix_transform.hpp>
#include <gtx/matrix_decompose.hpp>
const glm::vec2 nullVec = glm::vec2(-HUGE_VALF, -HUGE_VALF);
struct camImpl
{
	glm::vec3 pos;
	glm::vec3 target;
	bool dirtyBit;
	glm::mat4 view;

	glm::vec2 initialRotateMouse = nullVec;
	glm::vec3 rotateTarget;

	glm::vec2 initialTranslateMouse = nullVec;

};

void Camera::updateCam()
{
	if (pimpl->dirtyBit) {
		pimpl->view = glm::lookAt(pimpl->pos, pimpl->target, glm::vec3(0, 1, 0));
		pimpl->dirtyBit = false;
	}
}

Camera::Camera()
{
	pimpl = std::make_unique<camImpl>();
	pimpl->pos = glm::vec3(1);
	pimpl->target = glm::vec3(0);
	pimpl->rotateTarget = glm::vec3(0);
	pimpl->dirtyBit = true;
}


Camera::~Camera()
{
}

void Camera::setViewMatrix(shaderID sid)
{
	updateCam();
	ShaderManager::getShader(sid)->setMat4("view", pimpl->view);
}

void Camera::pan(float dx, float dy)
{
	pimpl->dirtyBit = true;
	const glm::vec3 lookDir = pimpl->pos - pimpl->target;
	const glm::vec3 moveDir = glm::normalize(glm::cross(glm::normalize(lookDir), glm::vec3(0, 1, 0)));
	const float sensitivityConstant = 0.01f;
	dx *= sensitivityConstant;
	dy *= sensitivityConstant;
	pimpl->pos.y += dy;
	pimpl->pos += moveDir * dx;
	pimpl->target.y += dy;
	pimpl->target += moveDir * dx;

}

void Camera::zoom(float dz)
{
	const glm::vec3 dir = pimpl->pos - pimpl->target;
	pimpl->pos += dz * glm::normalize(dir);
	pimpl->dirtyBit = true;
}

void Camera::rotate(float dx, float dy)
{
	pimpl->dirtyBit = true;
	glm::mat4 model;
	model = glm::rotate(model, glm::radians(dx), glm::vec3(0, 1, 0));
	const glm::vec3 lookDir = pimpl->target - pimpl->pos;
	const glm::vec3 axis = glm::cross(glm::normalize(lookDir), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(dy), axis);
	pimpl->pos = glm::vec3(model * glm::vec4(pimpl->pos, 1.0));
//	pimpl->target = glm::vec3(model * glm::vec4(pimpl->target, 1.0));
}

void Camera::notify(const command & msg)
{
	switch (msg.cmd) {
	case msg::cm_pan:
	{
		const glm::vec2 m2 = { (int)msg.args[0], (int)msg.args[1] };
		const glm::vec2 initialMouse = pimpl->initialTranslateMouse;
		if (initialMouse != nullVec) { //check to make sure vector will be valid
			glm::vec2 dir = glm::normalize(m2 - initialMouse);
			pan(dir.x, dir.y);
		}
		pimpl->initialTranslateMouse = m2;
		break;
	}
	case msg::cm_rotate:
	{
		const glm::vec2 m2 = { (int)msg.args[0], (int)msg.args[1] };
		const glm::vec2 initialMouse = pimpl->initialRotateMouse;
		if (initialMouse != nullVec) { //check to make sure vector will be valid
			glm::vec2 dir = glm::normalize(m2 - initialMouse);
			rotate(-dir.x, -dir.y);
		}
		pimpl->initialRotateMouse = m2;
		break;
	}
	case msg::cm_zoom:
		zoom(-(int)msg.args[0]);
		break;
	}
}

bool Camera::isInterested(msg m)
{
	return m < msg::cm_msgs;
}
