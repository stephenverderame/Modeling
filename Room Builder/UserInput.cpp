#include "UserInput.h"
#include <vector>
#include "Window.h"
#include <Windowsx.h>

struct uiImpl
{
	std::vector<std::reference_wrapper<Observer>> observers;
	bool mButtonDown;

	bool translate, rotate, scale;
	int boundAxis;
	bool snapMode;

	bool guiShow;

	int scrWidth, scrHeight;

	float lastTheta;
	float lastDist;
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
	pimpl->guiShow = false;
	pimpl->scrWidth = 1920;
	pimpl->scrHeight = 1080;
	pimpl->rotate = false;
	pimpl->lastTheta = FLT_MAX;
	pimpl->lastDist = FLT_MAX;
	pimpl->scale = false;
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
		cmd.cmd = pimpl->guiShow ? msg::gui_scroll : msg::cm_zoom;
		cmd.args[0] = (void*)(GET_WHEEL_DELTA_WPARAM(msg.wparam) < 0 ? -1 : 1);
		break;
	case WM_MBUTTONDOWN:
		pimpl->mButtonDown = true;
		break;
	case WM_MBUTTONUP:
		pimpl->mButtonDown = false;
		break;
	case WM_MOUSEMOVE:
		if (pimpl->mButtonDown && !pimpl->guiShow) {
			cmd.args[0] = (void*)(GET_X_LPARAM(msg.lparam));
			cmd.args[1] = (void*)(GET_Y_LPARAM(msg.lparam));
			cmd.cmd = Window::isKeyPress(keyCode::shift) ? msg::cm_pan : msg::cm_rotate;
		}
		else if (pimpl->translate && !pimpl->guiShow) {
			cmd.args[0] = (void*)(GET_X_LPARAM(msg.lparam));
			cmd.args[1] = (void*)(GET_Y_LPARAM(msg.lparam));
			cmd.args[2] = (void*)(pimpl->boundAxis);
			cmd.args[3] = (void*)(pimpl->snapMode);
			cmd.cmd = msg::cm_translateObj;
		}
		else if (pimpl->rotate && !pimpl->guiShow) {
			float x = GET_X_LPARAM(msg.lparam);
			float y = GET_Y_LPARAM(msg.lparam);
			float xc = pimpl->scrWidth / 2.0;
			float yc = pimpl->scrHeight / 2.0;
			x -= xc;
			y -= yc;
			float theta = atan2(x, y);
			if (pimpl->lastTheta == FLT_MAX) {
				pimpl->lastTheta = theta;
				break;
			}
			if (theta < 0) theta = 2 * 3.14159 + theta;
			float t2 = theta - pimpl->lastTheta;
			pimpl->lastTheta = theta;
			cmd.args[0] = &t2;
			cmd.args[1] = (void*)pimpl->boundAxis;
			cmd.cmd = msg::cm_rotateObj;
			notifyInterestedObservers(cmd);
			return;
		}
		else if (pimpl->scale && !pimpl->guiShow) {
			float x = GET_X_LPARAM(msg.lparam);
			float y = GET_Y_LPARAM(msg.lparam);
			float xc = pimpl->scrWidth / 2.0;
			float yc = pimpl->scrHeight / 2.0;
			float dst = sqrt((x - xc) * (x - xc) + (y - yc) * (y - yc));
			if (pimpl->lastDist == FLT_MAX) {
				pimpl->lastDist = dst;
				break;
			}
			float scale = dst / pimpl->lastDist;
			pimpl->lastDist = dst;
			cmd.cmd = msg::sn_scale;
			cmd.args[0] = &scale;
			cmd.args[1] = (void*)pimpl->boundAxis;
			notifyInterestedObservers(cmd);
			return;
		}
		else if (pimpl->guiShow) {
			float ndc[2] = { GET_X_LPARAM(msg.lparam) / (float)pimpl->scrWidth, GET_Y_LPARAM(msg.lparam) / (float)pimpl->scrHeight };
			cmd.cmd = msg::gui_mouseHover;
			cmd.args[0] = ndc;
			notifyInterestedObservers(cmd);
			return;
		}
		break;
	case WM_MOUSELEAVE:
		pimpl->mButtonDown = false;
		break;
	case WM_LBUTTONDOWN:
		if (pimpl->guiShow) {
			cmd.cmd = msg::gui_click;
			float ndc[2] = { GET_X_LPARAM(msg.lparam) / (float)pimpl->scrWidth, GET_Y_LPARAM(msg.lparam) / (float)pimpl->scrHeight };
			cmd.args[0] = ndc;
			notifyInterestedObservers(cmd);
			return;
		}
		else {
			cmd.cmd = msg::click;
			cmd.args[0] = (void*)(GET_X_LPARAM(msg.lparam));
			cmd.args[1] = (void*)(GET_Y_LPARAM(msg.lparam));
		}
		if (pimpl->translate || pimpl->rotate || pimpl->scale) {
			pimpl->translate = false;
			pimpl->rotate = false;
			pimpl->scale = false;
			Window::setCursor(cursorType::normal);
		}
		break;
	case WM_RBUTTONDOWN:
		cmd.cmd = msg::sn_deselect;
		pimpl->translate = false;
		pimpl->rotate = false;
		pimpl->scale = false;
		Window::setCursor(cursorType::normal);
		break;
	case WM_SIZE:
		cmd.cmd = msg::cm_wndSize;
		cmd.args[0] = (void*)(LOWORD(msg.lparam));
		cmd.args[1] = (void*)(HIWORD(msg.lparam));
		pimpl->scrWidth = LOWORD(msg.lparam);
		pimpl->scrHeight = HIWORD(msg.lparam);
		break;
	case WM_KEYDOWN:
	{
		int prevKeyState = msg.lparam >> 30;
		if (prevKeyState == 0) {//first press
			switch (msg.wparam) {
			case (int)keyCode::g:
				pimpl->translate = !pimpl->translate;
				pimpl->boundAxis = -1;
				Window::setCursor(pimpl->translate ? cursorType::move : cursorType::normal);
				break;
			case (int)keyCode::r:
				pimpl->rotate = !pimpl->rotate;
				pimpl->boundAxis = -1;
				Window::setCursor(pimpl->rotate ? cursorType::cross : cursorType::normal);
				break;
			case (int)keyCode::x:
				if (pimpl->translate || pimpl->rotate || pimpl->scale) {
					pimpl->boundAxis = 'x';
					if(pimpl->translate) Window::setCursor(cursorType::move_lr);
				}
				else {
					cmd.cmd = msg::sn_delete;
				}
				break;
			case (int)keyCode::y:
				if (pimpl->translate || pimpl->rotate || pimpl->scale) {
					pimpl->boundAxis = 'y';
					if (pimpl->translate) Window::setCursor(cursorType::move_ud);
				}
				break;
			case (int)keyCode::z:
				if (pimpl->translate || pimpl->rotate || pimpl->scale) {
					pimpl->boundAxis = 'z';
					if (pimpl->translate) Window::setCursor(cursorType::move_lr);
				}
				break;
			case (int)keyCode::s:
				if(Window::isKeyPress(keyCode::shift)) pimpl->snapMode = !pimpl->snapMode;
				else {
					pimpl->scale = !pimpl->scale;
					pimpl->boundAxis = -1;
					pimpl->lastDist = FLT_MAX;
					Window::setCursor(pimpl->scale ? cursorType::scale : cursorType::normal);
				}
				break;
			case (int)keyCode::a:
				if (Window::isKeyPress(keyCode::shift)) {
					cmd.cmd = msg::gui_addDialog;
					pimpl->guiShow = !pimpl->guiShow;
					Window::setCursor(cursorType::normal);
				}
				break;
			case (int)keyCode::leftArrow:
			case (int)keyCode::rightArrow:
				cmd.cmd = msg::sn_next;
				cmd.args[0] = (void*)(msg.wparam - (int)keyCode::leftArrow);
				break;
			}
		}
		if (pimpl->guiShow && cmd.cmd == msg::nill) {
			cmd.cmd = msg::gui_keydown;
			cmd.args[0] = (void*)msg.wparam;
			notifyInterestedObservers(cmd);
			return;
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
