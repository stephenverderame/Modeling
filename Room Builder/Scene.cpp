#include "Scene.h"
#include "Object.h"
#include <vector>
#include "RenderTarget.h"

struct sImpl
{
	std::vector<std::shared_ptr<Object>> objs;
	RenderTarget * rt;
};
struct msImpl
{
	std::vector<glm::vec3> objRealPos; //keeps track of objects' unsnapped positions
	std::unique_ptr<Rect> grid;
	std::unique_ptr<InstancedObject> gridDecorator;
	size_t selectedObjectIndex;
};
const float MainScene::BASE_UNIT = 0.5;
Scene::Scene(RenderTarget & rt)
{
	pimpl = std::make_unique<sImpl>();
	pimpl->rt = &rt;
}
MainScene::MainScene(RenderTarget & rt) : Scene(rt)
{
	mpimpl = std::make_unique<msImpl>();
	glm::vec3 offsets[10000];
	memset(offsets, 0, sizeof(offsets));
	for (int i = 0; i < 10000; ++i) {
		int x = i % 100 - 50;
		int y = i / 100 - 50;
		offsets[i] = { x * BASE_UNIT, 0, y * BASE_UNIT };
	}
	mpimpl->grid = std::make_unique<Rect>(true);
	mpimpl->grid->scale(glm::vec3(0.5));
	mpimpl->gridDecorator = std::make_unique<InstancedObject>(offsets, 10000);
	mpimpl->grid->addDecorator(*mpimpl->gridDecorator);
	mpimpl->selectedObjectIndex = ~0;
}


Scene::~Scene() = default;
MainScene::~MainScene() = default;

void Scene::addObject(std::shared_ptr<Object> & obj)
{
	pimpl->objs.push_back(obj);
	nvi_addObject(obj);
}

void Scene::addObject(std::shared_ptr<class Object>&& obj)
{
	pimpl->objs.push_back(obj);
	nvi_addObject(obj);
}

void MainScene::nvi_addObject(std::shared_ptr<class Object> obj)
{
	mpimpl->objRealPos.push_back(obj->getPos());
}

void Scene::renderScene()
{
	pimpl->rt->bindForWriting();
	renderScenePreconditions();
	for (auto o : pimpl->objs)
		o->draw();
	renderScenePostconditions();
	pimpl->rt->unBind();
}

void MainScene::renderScenePreconditions()
{
	mpimpl->grid->draw();
}
void MainScene::notify(const command & cmd)
{
	if (cmd.cmd == msg::sn_select) {
		glm::vec3 pos = glm::vec3(*reinterpret_cast<glm::vec4*>(cmd.args[0]));
		glm::vec3 camPos = *reinterpret_cast<glm::vec3*>(cmd.args[1]);
		glm::vec3 closest = pimpl->objs[0]->getPos();
		mpimpl->selectedObjectIndex = 0;
		for (size_t i = 0; i < pimpl->objs.size(); ++i) {
			auto o = pimpl->objs[i];
			float obj = abs(glm::dot(o->getPos() - camPos, pos - camPos) / (glm::length(o->getPos() - camPos) * glm::length(pos - camPos)));
			float close = abs(glm::dot(closest - camPos, pos - camPos) / (glm::length(closest - camPos) * glm::length(pos - camPos)));
			if (obj > close) {
				closest = o->getPos();
				mpimpl->selectedObjectIndex = i;
			}
		}
		for (auto o : pimpl->objs) {
			o->select(false);
		}
		if (mpimpl->selectedObjectIndex != ~0) pimpl->objs[mpimpl->selectedObjectIndex]->select(true);
	}
	else if (cmd.cmd == msg::sn_translateObj && mpimpl->selectedObjectIndex != ~0) {
		glm::vec3 dp = glm::vec3(*reinterpret_cast<glm::vec3*>(cmd.args[0]));
		bool snap = reinterpret_cast<bool>(cmd.args[1]);
		if (mpimpl->selectedObjectIndex != ~0) {
			pimpl->objs[mpimpl->selectedObjectIndex]->translate(-dp);
			mpimpl->objRealPos[mpimpl->selectedObjectIndex] -= dp;
			if (snap) {
				auto p = mpimpl->objRealPos[mpimpl->selectedObjectIndex];
				p.x = round(p.x / BASE_UNIT) * BASE_UNIT;
				p.y = round(p.y / BASE_UNIT) * BASE_UNIT;
				p.z = round(p.z / BASE_UNIT) * BASE_UNIT;
				pimpl->objs[mpimpl->selectedObjectIndex]->setPos(p);
			}
		}
	}
}
void Scene::compose()
{
	pimpl->rt->bindForReading();
}
bool MainScene::isInterested(msg m)
{
	return m > msg::cm_msgs && m < msg::sn_msgs;
}
