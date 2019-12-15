#include "Scene.h"
#include "Object.h"
#include <vector>

struct sImpl
{
	std::vector<std::shared_ptr<Object>> objs;
	std::unique_ptr<Rect> grid;
	std::unique_ptr<InstancedObject> gridDecorator;
	size_t selectedObjectIndex;
};

Scene::Scene()
{
	pimpl = std::make_unique<sImpl>();
	glm::vec3 offsets[10000];
	memset(offsets, 0, sizeof(offsets));
	for (int i = 0; i < 10000; ++i) {
		int x = i % 100 - 50;
		int y = i / 100 - 50;
		offsets[i] = { x * 0.5, 0, y * 0.5 };
	}
	pimpl->grid = std::make_unique<Rect>(true);
	pimpl->grid->scale(glm::vec3(0.5));
	pimpl->gridDecorator = std::make_unique<InstancedObject>(offsets, 10000);
	pimpl->grid->addDecorator(*pimpl->gridDecorator);
	pimpl->selectedObjectIndex = ~0;
}


Scene::~Scene() = default;

void Scene::addObject(std::shared_ptr<Object> & obj)
{
	pimpl->objs.push_back(obj);
}

void Scene::addObject(std::shared_ptr<class Object>&& obj)
{
	pimpl->objs.push_back(obj);
}

void Scene::renderScene()
{
	pimpl->grid->draw();
	for (auto o : pimpl->objs)
		o->draw();
}
void Scene::notify(const command & cmd)
{
	if (cmd.cmd == msg::sn_select) {
		glm::vec3 pos = glm::vec3(*reinterpret_cast<glm::vec4*>(cmd.args[0]));
		glm::vec3 camPos = *reinterpret_cast<glm::vec3*>(cmd.args[1]);
		glm::vec3 closest = pimpl->objs[0]->getPos();
		pimpl->selectedObjectIndex = 0;
		for (size_t i = 0; i < pimpl->objs.size(); ++i) {
			auto o = pimpl->objs[i];
			float obj = abs(glm::dot(o->getPos() - camPos, pos - camPos) / (glm::length(o->getPos() - camPos) * glm::length(pos - camPos)));
			float close = abs(glm::dot(closest - camPos, pos - camPos) / (glm::length(closest - camPos) * glm::length(pos - camPos)));
			if (obj > close) {
				closest = o->getPos();
				pimpl->selectedObjectIndex = i;
			}
		}
		for (auto o : pimpl->objs) {
			o->select(false);
		}
		if (pimpl->selectedObjectIndex != ~0) pimpl->objs[pimpl->selectedObjectIndex]->select(true);
	}
	else if (cmd.cmd == msg::sn_translateObj) {
		glm::vec3 dp = glm::vec3(*reinterpret_cast<glm::vec3*>(cmd.args[0]));
		if (pimpl->selectedObjectIndex != ~0) pimpl->objs[pimpl->selectedObjectIndex]->translate(-dp);
	}
}
bool Scene::isInterested(msg m)
{
	return m > msg::cm_msgs && m < msg::sn_msgs;
}
