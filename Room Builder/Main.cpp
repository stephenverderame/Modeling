#include "Window.h"
#include "ShaderManager.h"
#include "Object.h"
#include "Camera.h"
#include "UserInput.h"
#include "Scene.h"
int main() {
	Window wnd("Project", 1920, 1080);
	wnd.show(windowVisibility::show);
	auto cube = std::make_shared<Cube>();
	Camera cam;
	UserInput ui;
	wnd.attach(ui);
	ui.attach(cam);
	Scene mainScene;
	mainScene.addObject(cube);
	cam.attach(mainScene);
	auto c2 = std::make_shared<Cube>();
	c2->translate(glm::vec3(-5, 0, 5));
	mainScene.addObject(c2);
	int i = 0;
	while (true) {
		wnd.pollEvents();
		wnd.clear();
		cam.setViewMatrix(shaderID::basic);
		cam.setViewMatrix(shaderID::instance);
		mainScene.renderScene();
		wnd.switchBuffers();
	}
	return 0;
}