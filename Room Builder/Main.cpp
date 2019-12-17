#include "Window.h"
#include "ShaderManager.h"
#include "Object.h"
#include "Camera.h"
#include "UserInput.h"
#include "Scene.h"
#include "Compositor.h"
#include "RenderTarget.h"
#include "Font.h"
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
	Scene guiScene(renderGUI);
	mainScene.addObject(cube);
	cam.attach(mainScene);
	auto c2 = std::make_shared<Cube>();
	c2->translate(glm::vec3(-5, 0, 5));
	mainScene.addObject(c2);
	Compositor comp(1920, 1080);
	wnd.attach(comp);
	auto f = std::make_shared<Font>("C:\\Users\\stephen\\Desktop\\coding stuff\\Fonts\\SignedDistanceCS.fnt");
	auto test = std::make_shared<Text>(f);
	test->setColor(glm::vec4(1.0));
	test->setPos(glm::vec3(100, 100, 1.0));
	test->setText("Hello World");
	guiScene.addObject(test);
	comp.compose(mainScene);
	comp.compose(guiScene);
	while (true) {
		wnd.pollEvents();
		wnd.clear();
		cam.setViewMatrix(shaderID::basic);
		cam.setViewMatrix(shaderID::instance);
		mainScene.renderScene();
		guiScene.renderScene();
		comp.renderFinal();
		wnd.switchBuffers();
	}
	return 0;
}