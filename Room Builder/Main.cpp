#include "Window.h"
#include "ShaderManager.h"
#include "Object.h"
#include "Camera.h"
#include "UserInput.h"
int main() {
	Window wnd("Project", 1920, 1080);
	wnd.show(windowVisibility::show);
	Cube cube;
	Camera cam;
	UserInput ui;
	wnd.attach(ui);
	ui.attach(cam);
	glm::vec3 offsets[10000];
	memset(offsets, 0, sizeof(offsets));
	for (int i = 0; i < 10000; ++i) {
		int x = i % 100 - 50;
		int y = i / 100 - 50;
		offsets[i] = { x * 0.5, 0, y * 0.5 };
	}
	printf("Setup\n");
	Rect r(true);
	r.scale(glm::vec3(0.5));
	InstancedObject grid(offsets, 10000);
	r.addDecorator(grid);
	int i = 0;
	while (true) {
		wnd.pollEvents();
		wnd.clear();
		cam.setViewMatrix(shaderID::basic);
		cam.setViewMatrix(shaderID::instance);
		cube.draw();
		r.draw();
		wnd.switchBuffers();
	}
	return 0;
}