#include "UserInput.h"
#include <vector>
#include "Window.h"
#include <Windowsx.h>

struct uiImpl
{
	std::vector<std::reference_wrapper<Observer>> observers;
	bool mButtonDown;
};

void UserInput::notifyInterestedObservers(const command & cmd)
{
	for (auto o : pimpl->observers) {
		if (o.get().isInterested(cmd.cmd)) {
			o.get().notify(cmd);
		}
	}
}

UserInput::UserInput()
{
	pimpl = std::make_unique<uiImpl>();
}


UserInput::~UserInput()
{
}

void UserInput::notify(const message & msg)
{
	command cmd;
	cmd.cmd = msg::nill;
	switch (msg.msg) {
	case WM_MOUSEWHEEL:
		cmd.cmd = msg::cm_zoom;
		cmd.args[0] = (void*)(GET_WHEEL_DELTA_WPARAM(msg.wparam) < 0 ? -1 : 1);
		break;
	case WM_MBUTTONDOWN:
		pimpl->mButtonDown = true;
		break;
	case WM_MBUTTONUP:
		pimpl->mButtonDown = false;
		break;
	case WM_MOUSEMOVE:
		if (pimpl->mButtonDown) {
			cmd.args[0] = (void*)(GET_X_LPARAM(msg.lparam));
			cmd.args[1] = (void*)(GET_Y_LPARAM(msg.lparam));
			cmd.cmd = Window::isKeyPress(keyCode::shift) ? msg::cm_pan : msg::cm_rotate;
		}
		break;
	case WM_MOUSELEAVE:
		printf("leave\n");
		pimpl->mButtonDown = false;
		break;
	}
	notifyInterestedObservers(cmd);
}

void UserInput::attach(Observer & observer)
{
	pimpl->observers.emplace_back(observer);
}
