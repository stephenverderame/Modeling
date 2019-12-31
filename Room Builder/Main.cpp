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
	mirror->addRequiredPass(renderPass::reflection);
	GuiScene guiScene(renderGUI, f, mainScene);
	mainScene.addObject(cube);
	mainScene.addObject(m);
	mainScene.addObject(mirror);
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
		wnd.pollEvents();
		wnd.clear();
		glm::vec3 cp;
		cam.getPos(cp.x, cp.y, cp.z);
		mainScene.handlePasses(cp.x, cp.y, cp.z);
		cam.setViewMatrix(shaderID::basic);
		cam.setViewMatrix(shaderID::instance);
		mainScene.renderScene();
		guiScene.renderScene();
		comp.renderFinal();
		wnd.switchBuffers();
	}
	return 0;
}