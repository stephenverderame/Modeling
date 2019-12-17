#include "UserInput.h"
#include <vector>
#include "Window.h"
#include <Windowsx.h>

struct uiImpl
{
	std::vector<std::reference_wrapper<Observer>> observers;
	bool mButtonDown;

	bool translate;
	int boundAxis;
	bool snapMode;
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
	pimpl->mButtonDown = false;
	pimpl->translate = false;
	pimpl->boundAxis = -1;
	pimpl->snapMode = false;
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
		else if (pimpl->translate) {
			cmd.args[0] = (void*)(GET_X_LPARAM(msg.lparam));
			cmd.args[1] = (void*)(GET_Y_LPARAM(msg.lparam));
			cmd.args[2] = (void*)(pimpl->boundAxis);
			cmd.args[3] = (void*)(pimpl->snapMode);
			cmd.cmd = msg::cm_translateObj;
		}
		break;
	case WM_MOUSELEAVE:
		pimpl->mButtonDown = false;
		break;
	case WM_LBUTTONDOWN:
		cmd.cmd = msg::click;
		cmd.args[0] = (void*)(GET_X_LPARAM(msg.lparam));
		cmd.args[1] = (void*)(GET_Y_LPARAM(msg.lparam));
		if (pimpl->translate) pimpl->translate = false;
		break;
	case WM_SIZE:
		cmd.cmd = msg::cm_wndSize;
		cmd.args[0] = (void*)(LOWORD(msg.lparam));
		cmd.args[1] = (void*)(HIWORD(msg.lparam));
		break;
	case WM_KEYDOWN:
	{
		int prevKeyState = msg.lparam >> 30;
		if (prevKeyState == 0) {//first press
			switch (msg.wparam) {
			case (int)keyCode::g:
				pimpl->translate = !pimpl->translate;
				pimpl->boundAxis = -1;
				break;
			case (int)keyCode::x:
				if(pimpl->translate) pimpl->boundAxis = 'x';
				break;
			case (int)keyCode::y:
				if (pimpl->translate) pimpl->boundAxis = 'y';
				break;
			case (int)keyCode::z:
				if (pimpl->translate) pimpl->boundAxis = 'z';
				break;
			case (int)keyCode::s:
				pimpl->snapMode = !pimpl->snapMode;
				break;
			}

		}
		break;
	}
	}
	notifyInterestedObservers(cmd);
}

void UserInput::attach(Observer & observer)
{
	pimpl->observers.emplace_back(observer);
}
