#include "Camera.h"
#include "ShaderManager.h"
#include "Shader.h"
#include <glm.hpp>
#include "Window.h"
#include <gtc/matrix_transform.hpp>
#include <gtx/matrix_decompose.hpp>
#include <vector>
const glm::vec2 nullVec = glm::vec2(-HUGE_VALF, -HUGE_VALF);
struct camImpl
{
	glm::vec3 pos;
	glm::vec3 target;
	bool dirtyBit;
	glm::mat4 view;
	glm::mat4 proj;

	glm::vec2 initialRotateMouse = nullVec;
	glm::vec3 rotateTarget;

	glm::vec2 initialTranslateMouse = nullVec;

	std::vector<std::reference_wrapper<Observer>> obs;
	glm::vec2 screenDims;

	glm::vec2 initialObjTransformMouse = nullVec;

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
	pimpl->proj = glm::perspective(glm::radians(45.f), 1920.f / 1080.f, 0.1f, 100.f);
	pimpl->screenDims = glm::vec2(1920, 1080);
}


Camera::~Camera()
{
}

void Camera::setViewMatrix(shaderID sid)
{
	updateCam();
	ShaderManager::getShader(sid)->setMat4("view", pimpl->view);
	ShaderManager::getShader(sid)->setMat4("projection", pimpl->proj);
}

void Camera::pan(float dx, float dy)
{
	pimpl->dirtyBit = true;
	const glm::vec3 lookDir = pimpl->pos - pimpl->target;
	const glm::vec3 moveDir = glm::normalize(glm::cross(glm::normalize(lookDir), glm::vec3(0, 1, 0)));
	const glm::vec3 yDir = -glm::normalize(glm::cross(glm::normalize(lookDir), moveDir));
	const float sensitivityConstant = glm::max(glm::distance(pimpl->pos, pimpl->target) * 0.01, 0.0001);
	dx *= sensitivityConstant;
	dy *= sensitivityConstant;
	pimpl->pos += yDir * dy;
	pimpl->pos += moveDir * dx;
	pimpl->target += yDir * dy;
	pimpl->target += moveDir * dx;

}

void Camera::zoom(float dz)
{
	const glm::vec3 dir = pimpl->pos - pimpl->target;
	const glm::vec3 zoom = dz * glm::normalize(dir);
	if (!(glm::length(zoom) > glm::length(dir) && glm::dot(zoom, dir) == glm::length(zoom) * glm::length(dir))) {
		pimpl->pos += zoom;
	}
	pimpl->dirtyBit = true;
}

void Camera::rotate(float dx, float dy)
{
	if (glm::distance(pimpl->pos, pimpl->target) > 0.01 && abs(dx + dy) > 0 && abs(dx + dy) < MAXSHORT) {
		pimpl->dirtyBit = true;
		const float sensitivity = glm::min(glm::distance(pimpl->pos, pimpl->target), 1.0f);
		glm::mat4 model;
		model = glm::translate(model, pimpl->target);
		model = glm::rotate(model, glm::radians(dx * sensitivity), glm::vec3(0, 1, 0));
		model = glm::translate(model, -pimpl->target);
		const glm::vec3 lookDir = pimpl->target - pimpl->pos;
		const glm::vec3 axis = glm::cross(glm::normalize(lookDir), glm::vec3(0, 1, 0));
		model = glm::translate(model, -pimpl->pos);
		model = glm::rotate(model, glm::radians(dy * sensitivity), axis);
		model = glm::translate(model, pimpl->pos);
		pimpl->pos = glm::vec3(model * glm::vec4(pimpl->pos, 1.0));
		//	pimpl->target = glm::vec3(model * glm::vec4(pimpl->target, 1.0));
	}
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
	case msg::cm_wndSize:
		pimpl->proj = glm::perspective(glm::radians(45.f), (float)reinterpret_cast<int>(msg.args[0]) / reinterpret_cast<int>(msg.args[1]), 0.1f, 100.f);
		pimpl->screenDims = glm::vec2(reinterpret_cast<int>(msg.args[0]), reinterpret_cast<int>(msg.args[1]));
		break;
	case msg::click:
	{
		float mx = reinterpret_cast<int>(msg.args[0]);
		float my = reinterpret_cast<int>(msg.args[1]);
		glm::mat4 inv = pimpl->proj * pimpl->view;
		inv = glm::inverse(inv);
		glm::vec4 p;
		p.x = (2.0f * (mx / pimpl->screenDims.x)) - 1.f;
		p.y = (2.0f * (my / pimpl->screenDims.y)) - 1.f;
		p.z = 2.0 * pimpl->pos.z - 1.0;
		p.w = 1.0;
		p = inv * p;
		p.w = 1.0 / p.w;
		p.x *= p.w;
		p.y *= p.w;
		p.z *= p.w;
		command cmd;
		cmd.args[0] = &p;
		cmd.args[1] = &pimpl->pos;
		glm::vec3 lookDir = pimpl->pos - pimpl->target;
		cmd.args[2] = &lookDir;
		cmd.cmd = msg::sn_select;
		for (auto& o : pimpl->obs) {
			if (o.get().isInterested(cmd.cmd))
				o.get().notify(cmd);
		}
		break;
	}
	case msg::cm_translateObj:
	{
		const glm::vec2 m = { (int)msg.args[0], (int)msg.args[1] };
		if (pimpl->initialObjTransformMouse != nullVec) {
			glm::vec2 d = glm::normalize(m - pimpl->initialObjTransformMouse);
			const float sensitivityConstant = glm::max(glm::distance(pimpl->pos, pimpl->target) * 0.01, 0.0001);
			d.x *= sensitivityConstant;
			d.y *= sensitivityConstant;
			glm::vec3 dP;
			int axis = (int)msg.args[2];
			if (axis == -1) {
				const glm::vec3 lookDir = pimpl->pos - pimpl->target;
				const glm::vec3 moveDir = glm::normalize(glm::cross(glm::normalize(lookDir), glm::vec3(0, 1, 0)));
				const glm::vec3 yDir = -glm::normalize(glm::cross(glm::normalize(lookDir), moveDir));
				dP = moveDir * d.x + yDir * d.y;
			}
			else if (axis == 'x') {
				dP = glm::vec3(d.x, 0, 0);
			}
			else if (axis == 'y') {
				dP = glm::vec3(0, d.y, 0);
			}
			else dP = glm::vec3(0, 0, d.x);
			command cmd;
			cmd.args[0] = &dP;
			cmd.cmd = msg::sn_translateObj;
			for (auto& o : pimpl->obs) {
				if (o.get().isInterested(cmd.cmd))
					o.get().notify(cmd);
			}
		}
		pimpl->initialObjTransformMouse = m;
		break;
	}

	}
}

bool Camera::isInterested(msg m)
{
	return m < msg::cm_msgs || m == msg::click;
}

void Camera::attach(Observer & ob)
{
	pimpl->obs.emplace_back(ob);
}
