#include "Scene.h"
#include "Object.h"
#include <vector>
#include "RenderTarget.h"
#include "Gui.h"
#include "Font.h"
#include <glad\glad.h>
#include "Shader.h"
#include "ShaderManager.h"
#include <gtx/matrix_decompose.hpp>

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

	std::vector<std::shared_ptr<RenderTarget2D>> reflectionTargets;
	int refTargets;
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
	mpimpl->reflectionTargets.push_back(std::make_shared<RenderTarget2D>(1920, 1080, 10));
}


Scene::~Scene() = default;
MainScene::~MainScene() = default;

void Scene::addObject(std::shared_ptr<Object> obj)
{
	pimpl->objs.push_back(obj);
	nvi_addObject(obj);
}

void MainScene::nvi_addObject(std::shared_ptr<class Object> obj)
{
	mpimpl->objRealPos.push_back(obj->getPos());
	if ((int)obj->getRequiredPasses() & (int)renderPass::reflection) {
		printf("Adding reflection target\n");
		mpimpl->reflectionTargets.push_back(std::make_shared<RenderTarget2D>(1920, 1080, 10));
	}
}

void Scene::renderScene(renderPass p)
{
	if(p == renderPass::standard) pimpl->rt->bindForWriting();
	renderScenePreconditions();
	for (auto o : pimpl->objs) {
		if(p == renderPass::standard) renderObjPreconditions(o);
		o->draw(p);
	}
	renderScenePostconditions();
	if(p == renderPass::standard) pimpl->rt->unBind();
}

void MainScene::renderScenePreconditions()
{
	mpimpl->grid->draw();
	mpimpl->refTargets = 0;
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
			//			pimpl->objs[mpimpl->selectedObjectIndex]->translate(-dp);
			mpimpl->objRealPos[mpimpl->selectedObjectIndex] -= dp;
			if (snap) {
				auto p = mpimpl->objRealPos[mpimpl->selectedObjectIndex];
				p.x = round(p.x / BASE_UNIT) * BASE_UNIT;
				p.y = round(p.y / BASE_UNIT) * BASE_UNIT;
				p.z = round(p.z / BASE_UNIT) * BASE_UNIT;
				pimpl->objs[mpimpl->selectedObjectIndex]->setPos(p);
			}
			else
				pimpl->objs[mpimpl->selectedObjectIndex]->setPos(mpimpl->objRealPos[mpimpl->selectedObjectIndex]);
		}
	}
	else if (cmd.cmd == msg::sn_delete && mpimpl->selectedObjectIndex != ~0) {
		pimpl->objs.erase(pimpl->objs.begin() + mpimpl->selectedObjectIndex);
		mpimpl->objRealPos.erase(mpimpl->objRealPos.begin() + mpimpl->selectedObjectIndex);
		mpimpl->selectedObjectIndex = ~0;
	}
	else if (cmd.cmd == msg::sn_rotateObj && mpimpl->selectedObjectIndex != ~0) {
		glm::vec4 rot = *reinterpret_cast<glm::vec4*>(cmd.args[0]);
		auto save = pimpl->objs[mpimpl->selectedObjectIndex]->getPos();
		pimpl->objs[mpimpl->selectedObjectIndex]->setPos(glm::vec3(0));
		pimpl->objs[mpimpl->selectedObjectIndex]->rotate(glm::vec3(rot), rot.w);
		pimpl->objs[mpimpl->selectedObjectIndex]->setPos(save);
	}
	else if (cmd.cmd == msg::sn_scale && mpimpl->selectedObjectIndex != ~0) {
		float scale = *reinterpret_cast<float*>(cmd.args[0]);
		int axis = (int)cmd.args[1];
		glm::vec3 vAxis = glm::vec3(1);
		vAxis *= scale;
		switch (axis) {
		case 'x':
			vAxis.y = 1.0;
			vAxis.z = 1.0;
			break;
		case 'y':
			vAxis.x = 1.0;
			vAxis.z = 1.0;
			break;
		case 'z':
			vAxis.x = 1.0;
			vAxis.y = 1.0;
			break;
		}
		pimpl->objs[mpimpl->selectedObjectIndex]->scale(vAxis);
	}
	else if (cmd.cmd == msg::sn_deselect && mpimpl->selectedObjectIndex != ~0) {
		pimpl->objs[mpimpl->selectedObjectIndex]->select(false);
		mpimpl->selectedObjectIndex = ~0;
	}
	else if (cmd.cmd == msg::sn_next) {
		int dir = (int)cmd.args[0];
		if (mpimpl->selectedObjectIndex != ~0) {
			mpimpl->selectedObjectIndex = (mpimpl->selectedObjectIndex + (dir - 1)) % pimpl->objs.size();
			for (size_t i = 0; i < pimpl->objs.size(); ++i) {
				pimpl->objs[i]->select(i == mpimpl->selectedObjectIndex ? true : false);
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
struct gImpl
{
	std::unique_ptr<GuiList> addDialog;
	std::unique_ptr<Object> objToAdd;

	std::unique_ptr<CompositeObject> dataDialog;
	std::shared_ptr<TextField> t1, t2, t3;
	std::shared_ptr<Button> btn;
	bool showData;
	Scene * sn;
};
GuiScene::GuiScene(RenderTarget & rt, std::shared_ptr<Font> f, class Scene & sn) : Scene(rt)
{
	gpimpl = std::make_unique<gImpl>();
	gpimpl->addDialog = std::make_unique<GuiList>(0.4, 0.4, 0.1, 0.5, f);
	gpimpl->addDialog->addItem("Cube");
	gpimpl->addDialog->addItem("Test");
	gpimpl->addDialog->setOnClick(std::bind(&GuiScene::clickCallback, this, std::placeholders::_1, std::placeholders::_2));
	font = f;

	gpimpl->dataDialog = std::make_unique<CompositeObject>();
	auto bg = std::make_shared<GuiRect>();
	bg->setPos(glm::vec3(0.5, 0.3999, 1.0));
	bg->setColor(glm::vec4(0.2, 0.2, 0.2, 0.8));
	bg->scale(glm::vec3(0.4, 0.5, 1.0));
	gpimpl->dataDialog->addObject(bg);
	gpimpl->t1 = std::make_shared<TextField>(0.5001, 0.8, 0.3, 0.05, f);
	gpimpl->t1->setLabel("Width: ");
	gpimpl->t2 = std::make_shared<TextField>(0.5001, 0.65, 0.3, 0.05, f);
	gpimpl->t2->setLabel("Height: ");
	gpimpl->t3 = std::make_shared<TextField>(0.5001, 0.5, 0.3, 0.05, f);
	gpimpl->t3->setLabel("Depth: ");
	gpimpl->btn = std::make_shared<Button>(0.6, 0.4, 0.1, 0.05, f);
	gpimpl->btn->setText("Add");
	gpimpl->btn->setCallback(std::bind(&GuiScene::addCallback, this));
	gpimpl->dataDialog->addObject(gpimpl->btn);
	gpimpl->dataDialog->addObject(gpimpl->t1);
	gpimpl->dataDialog->addObject(gpimpl->t2);
	gpimpl->dataDialog->addObject(gpimpl->t3);

	gpimpl->showData = false;
	gpimpl->sn = &sn;

}
GuiScene::~GuiScene() = default;

void GuiScene::clickCallback(int id, const char * name)
{
	if (strstr(name, "Cube")) {
		gpimpl->objToAdd = std::make_unique<Cube>();
		gpimpl->showData = !gpimpl->showData;
	}
}

void GuiScene::addCallback()
{
	float w = std::strtof(gpimpl->t1->getText().c_str(), nullptr);
	float h = std::strtof(gpimpl->t2->getText().c_str(), nullptr);
	float d = std::strtof(gpimpl->t3->getText().c_str(), nullptr);
	gpimpl->objToAdd->scale(glm::vec3(w, h, d));
	gpimpl->objToAdd->setPos(glm::vec3(0));
	gpimpl->showData = false;
	gpimpl->sn->addObject(std::move(gpimpl->objToAdd));
}

void GuiScene::notify(const command & cmd)
{
	switch (cmd.cmd) {
	case msg::gui_addDialog:
		gpimpl->addDialog->setVisible(!gpimpl->addDialog->isVisible());
		if (gpimpl->showData) gpimpl->showData = false;
		break;
	}
	gpimpl->addDialog->notify(cmd);
	gpimpl->t1->notify(cmd);
	gpimpl->t2->notify(cmd);
	gpimpl->t3->notify(cmd);
	gpimpl->btn->notify(cmd);
}

bool GuiScene::isInterested(msg m)
{
	return true;
}
void GuiScene::renderScenePreconditions()
{
	glDisable(GL_DEPTH_TEST);
	unsigned int w, h;
	pimpl->rt->getDimensions(w, h);
	ShaderManager::getShader(shaderID::gui)->setVec2i("screenSize", glm::ivec2(w, h));
	gpimpl->addDialog->draw();
	if (gpimpl->showData)
		gpimpl->dataDialog->draw();
}
void GuiScene::renderScenePostconditions()
{
	glEnable(GL_DEPTH_TEST);
}

void MainScene::handlePasses(float px, float py, float pz)
{
	ShaderManager::useShader(shaderID::basic);
	glm::vec3 playerPos = { px, py, pz };
	int j = 0;
	for (int i = 0; i < pimpl->objs.size(); ++i) {
		if ((int)pimpl->objs[i]->getRequiredPasses() & (int)renderPass::reflection) {
			glm::vec3 unused;
			glm::vec4 unused4;
			glm::quat rotation;
			glm::decompose(pimpl->objs[i]->getModel(), unused, rotation, unused, unused, unused4);
			glm::vec3 dir = rotation * glm::vec3(0, 1, 0);
			glm::mat4 view = glm::lookAt(pimpl->objs[i]->getPos(), playerPos/*pimpl->objs[i]->getPos() + dir*/, glm::vec3(0, 1, 0));
			ShaderManager::getShader(shaderID::basic)->setMat4("view", view);
			ShaderManager::getShader(shaderID::instance)->setMat4("view", view);
			mpimpl->reflectionTargets[j]->bindForWriting();
			renderScene(renderPass::reflection);
			mpimpl->reflectionTargets[j++]->unBind();
		}
	}
}
void MainScene::renderObjPreconditions(std::shared_ptr<class Object> obj)
{
	if ((int)obj->getRequiredPasses() & (int)renderPass::reflection)
	{
		ShaderManager::useShader(shaderID::basic);
		mpimpl->reflectionTargets[mpimpl->refTargets++]->bindForReading();
	}
}

