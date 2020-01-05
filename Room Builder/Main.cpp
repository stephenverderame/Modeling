#include "Window.h"
#include "ShaderManager.h"
#include "Object.h"
#include "Camera.h"
#include "UserInput.h"
#include "Scene.h"
#include "Compositor.h"
#include "RenderTarget.h"
#include "Font.h"
#include "Model.h"

#include "Shader.h"
#include <time.h>
#include "Light.h"

int main() {
	Window wnd("Project", 1920, 1080);
	wnd.show(windowVisibility::show);
	auto cube = std::make_shared<Cube>();
	Camera cam;
	UserInput ui;
	wnd.attach(ui);
	ui.attach(cam);
	MultisampledRenderTarget render3D(1920, 1080, 16);
	wnd.attach(render3D);
	MultisampledRenderTarget renderGUI(1920, 1080, 16);
	wnd.attach(renderGUI);
	MainScene mainScene(render3D);
	auto f = std::make_shared<Font>("C:\\Users\\stephen\\Desktop\\coding stuff\\Fonts\\SignedDistanceArial.fnt");
//	auto m = std::make_shared<Model>("C:\\Users\\stephen\\Downloads\\crysis-nano-suit-2\\everything\\scene.fbx");
	auto m = std::make_shared<Model>("C:\\Users\\stephen\\Desktop\\coding stuff\\Assets\\Queen_Bed\\Queen Bed.obj");
	auto mirror = std::make_shared<Model>("C:\\Users\\stephen\\Desktop\\coding stuff\\Assets\\Baker_Emerald_Mirror_fbx_obj\\Baker_Emerald_Mirror.obj");
//	auto m = std::make_shared<Model>("C:\\Users\\stephen\\Downloads\\Snooze_3DS\\Snooze 3DS.3ds");
	m->translate(glm::vec3(0));
//	m->scale(glm::vec3(0.001));
	m->scale(glm::vec3(0.01));
	mirror->scale(glm::vec3(0.01));
	mirror->addRequiredPass(RENDER_PASS_REFLECTION);
	GuiScene guiScene(renderGUI, f, mainScene);
	auto test = std::make_shared<Cube>();
	test->translate(glm::vec3(3));
	test->scale(glm::vec3(0.8));
	test->setShader(shaderID::fur);
	auto l = std::make_shared<Light>();
	l->translate(glm::vec3(0, 5, 0));
	auto floor = std::make_shared<Rect>();
	floor->rotate(glm::vec3(1, 0, 0), glm::radians(90.f));
	floor->scale(glm::vec3(20));
	mainScene.addObject(floor);
	mainScene.addObject(cube);
	mainScene.addObject(m);
	mainScene.addObject(mirror);
	mainScene.addObject(test);
	mainScene.addObject(l);
	ui.attach(guiScene);
	ui.attach(mainScene);
	cam.attach(mainScene);
	auto c2 = std::make_shared<Cube>();
	c2->translate(glm::vec3(-5, 0, 5));
	mainScene.addObject(c2);
	Compositor comp(1920, 1080);
	wnd.attach(comp);
	comp.compose(mainScene);
	comp.compose(guiScene);
	while (true) {
		ShaderManager::getShader(shaderID::fur)->setFloat("time", (float)clock() / CLOCKS_PER_SEC);
		wnd.pollEvents();
		wnd.clear();
		mainScene.handlePasses(cam);
		cam.setViewMatrix(shaderID::basic);
		cam.setViewMatrix(shaderID::instance);
		cam.setViewMatrix(shaderID::fur);
		mainScene.renderScene();
		guiScene.renderScene();
		comp.renderFinal();
		wnd.switchBuffers();
	}
	return 0;
}