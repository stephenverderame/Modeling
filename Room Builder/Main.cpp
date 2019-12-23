#include "Window.h"
#include "ShaderManager.h"
#include "Object.h"
#include "Camera.h"
#include "UserInput.h"
#include "Scene.h"
#include "Compositor.h"
#include "RenderTarget.h"
#include "Font.h"

#include "Gui.h"
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
	GuiScene guiScene(renderGUI, f, mainScene);
	mainScene.addObject(cube);
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
		cam.setViewMatrix(shaderID::basic);
		cam.setViewMatrix(shaderID::instance);
		mainScene.renderScene();
		guiScene.renderScene();
		comp.renderFinal();
		wnd.switchBuffers();
	}
	return 0;
}