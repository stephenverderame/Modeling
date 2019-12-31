#include "Window.h"
#include <Windows.h>
#include <glad/glad.h>
#include <time.h>
#include <vector>
#include "Observer.h"
#include <Windowsx.h>
#pragma region WGL
#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_DRAW_TO_BITMAP_ARB 0x2002
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NEED_PALETTE_ARB 0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB 0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB 0x2006
#define WGL_SWAP_METHOD_ARB 0x2007
#define WGL_NUMBER_OVERLAYS_ARB 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB 0x2009
#define WGL_TRANSPARENT_ARB 0x200A
#define WGL_SHARE_DEPTH_ARB 0x200C
#define WGL_SHARE_STENCIL_ARB 0x200D
#define WGL_SHARE_ACCUM_ARB 0x200E
#define WGL_SUPPORT_GDI_ARB 0x200F
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_STEREO_ARB 0x2012
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201A
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_ALPHA_SHIFT_ARB 0x201C
#define WGL_ACCUM_BITS_ARB 0x201D
#define WGL_ACCUM_RED_BITS_ARB 0x201E
#define WGL_ACCUM_GREEN_BITS_ARB 0x201F
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_GENERIC_ACCELERATION_ARB 0x2026
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_SWAP_EXCHANGE_ARB 0x2028
#define WGL_SWAP_COPY_ARB 0x2029
#define WGL_SWAP_UNDEFINED_ARB 0x202A
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_TYPE_COLORINDEX_ARB 0x202C
#define WGL_TRANSPARENT_RED_VALUE_ARB 0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042
#pragma endregion //WGL
struct winImpl
{
	std::vector<WindowObserver *> obs;
	std::vector<EventListener> commandListeners;
	std::vector<EventListener> eventListeners;
};
using wglChoosePixelFormatARB_PROC = BOOL(__stdcall*)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);
Window * Window::activeWindow = nullptr;
int Window::mouseX = 0;
int Window::mouseY = 0;
HCURSOR cursors[10];
cursorType Window::activeCursor = cursorType::normal;
Window::Window(const char * title, int width, int height) : hWnd(nullptr), hDc(nullptr), hRc(nullptr), errorCode(0)
{
	pimpl = std::make_unique<winImpl>();
	cursors[0] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	cursors[1] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));
	cursors[2] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS));
	cursors[3] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL));
	cursors[4] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
	cursors[5] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
	cursors[6] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENESW));
	cursors[7] = LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM));
	activeWindow = this;
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Window::windowProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "defWindowClass";
	RECT size = { 0, 0, width, height };
	AdjustWindowRect(&size, WS_OVERLAPPED, FALSE);
	RegisterClassEx(&wc);
	hWnd = CreateWindowEx(NULL, wc.lpszClassName, title, WS_OVERLAPPEDWINDOW | wc.style, 0, 0, size.right - size.left, size.bottom - size.top, NULL, NULL, NULL, NULL);
	if (hWnd == nullptr) errorCode = 100;
	else {
		hDc = GetDC(hWnd);

		PIXELFORMATDESCRIPTOR px;
		int ret = 0;
		ZeroMemory(&px, sizeof(px));
		px.nSize = sizeof(px);
		px.nVersion = 1;
		px.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		px.iPixelType = PFD_TYPE_RGBA;
		px.cColorBits = 32;
		//	px.cStencilBits = 8;
		px.cDepthBits = 32;
		px.cAlphaBits = 8;
		px.iLayerType = PFD_MAIN_PLANE;
		ret = ChoosePixelFormat(hDc, &px);
		if (!ret) errorCode = 200;
		else {
			if (SetPixelFormat(hDc, ret, &px) == 0) errorCode = 250;
			else {
				DescribePixelFormat(hDc, ret, sizeof(px), &px);

				hRc = wglCreateContext(hDc);
				if (hRc == NULL) errorCode = 300;
				else {
					if (wglMakeCurrent(hDc, hRc) == FALSE) errorCode = 400;
					else {
						auto ChoosePixelFormatARB = (wglChoosePixelFormatARB_PROC)wglGetProcAddress("wglChoosePixelFormatARB");
						if (ChoosePixelFormatARB == NULL) printf("Null!\n");
						const int attribList[] = {
							WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
							WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
							WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
							WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
							WGL_COLOR_BITS_ARB, 24,
							WGL_DEPTH_BITS_ARB, 24,
							WGL_ALPHA_BITS_ARB, 8,
							WGL_STENCIL_BITS_ARB, 8,
							WGL_SAMPLE_BUFFERS_ARB, 1,
							WGL_SAMPLES_ARB, 16,
							0
						};
						int pixelFormat;
						UINT numFormats;
						if (ChoosePixelFormatARB(hDc, attribList, NULL, 1, &pixelFormat, &numFormats) == FALSE) printf("False!");
						ReleaseDC(hWnd, hDc);
						wglDeleteContext(hRc);
						DestroyWindow(hWnd);
						hWnd = CreateWindowEx(NULL, wc.lpszClassName, title, WS_OVERLAPPEDWINDOW | wc.style, 0, 0, size.right - size.left, size.bottom - size.top, NULL, NULL, NULL, NULL);
						hDc = GetDC(hWnd);
						SetPixelFormat(hDc, pixelFormat, NULL);
						hRc = wglCreateContext(hDc);
						wglMakeCurrent(hDc, hRc);
						if (!gladLoadGL()) errorCode = 500;
						glEnable(GL_MULTISAMPLE);
						glEnable(GL_DEPTH_TEST);
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						SetProcessDPIAware();
					}
				}
			}
		}
	}
	TRACKMOUSEEVENT tm;
	tm.cbSize = sizeof(tm);
	tm.hwndTrack = activeWindow->hWnd;
	tm.dwFlags = TME_LEAVE;
	tm.dwHoverTime = 1;
	TrackMouseEvent(&tm);
}

int Window::getLastError()
{
	int e = errorCode;
	errorCode = 0;
	return e;
}


Window::~Window()
{
	if (hDc != nullptr) ReleaseDC(hWnd, hDc);
	if (hRc != nullptr) wglDeleteContext(hRc);
	if (hWnd != nullptr) DestroyWindow(hWnd);
}

bool Window::show(windowVisibility show)
{
	if (hWnd == nullptr) return false;
	ShowWindow(hWnd, (int)show);
	return true;
}

void Window::pollEvents(std::function<void(struct tagMSG &)> f)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		if (f != nullptr) f(msg);
		if (msg.message == WM_QUIT) quit = true;
		DispatchMessage(&msg);
	}
}

void Window::switchBuffers()
{
	SwapBuffers(hDc);
}

bool Window::shouldQuit()
{
	return quit;
}

bool Window::isKeyPress(keyCode k)
{
	return GetAsyncKeyState((int)k) < 0;
}
LRESULT __stdcall Window::windowProc(HWND window, UINT msg, WPARAM w, LPARAM l)
{
	for (auto e : activeWindow->pimpl->eventListeners) {
		if (e.eventId == msg) {
			if (e.callback(w, l)) return TRUE;
		}
	}
	message msg_ = { window, msg, w, l };
	for (auto o : activeWindow->pimpl->obs) {
		o->notify(msg_);
	}
	switch (msg) {
	case WM_SETCURSOR:
		SetCursor(cursors[(int)activeWindow->activeCursor]);
		return TRUE;
	case WM_MOUSEMOVE:
		mouseX = GET_X_LPARAM(l);
		mouseY = GET_Y_LPARAM(l);
		break;
	case WM_MBUTTONDOWN:
		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(tm);
		tm.hwndTrack = activeWindow->hWnd;
		tm.dwFlags = TME_LEAVE;
		tm.dwHoverTime = 1;
		TrackMouseEvent(&tm);
		break;
	case WM_COMMAND:
		for (auto it : activeWindow->pimpl->commandListeners) {
			if (it.eventId == LOWORD(w)) {
				if (it.callback(w, l)) return TRUE;
			}
		}
		break;
	case WM_SIZE:
		glViewport(0, 0, LOWORD(l), HIWORD(l));
		return TRUE;
	}
	return DefWindowProc(window, msg, w, l);

}
void Window::attach(class WindowObserver & ob)
{
	pimpl->obs.push_back(&ob);
}
void Window::addCommandListener(EventListener el)
{
	pimpl->commandListeners.push_back(el);
}
void Window::addEventListener(EventListener el)
{
	pimpl->eventListeners.push_back(el);
}
void Window::setCursor(cursorType c)
{
	activeCursor = c;
}

void Window::getMousePos(int & x, int & y)
{
	x = mouseX;
	y = mouseY;
}

void Window::clear()
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
char Window::keyToLetter(keyCode c)
{
	char k = 0;
	switch (c) {
	case keyCode::semicolon:
		k = Window::isKeyPress(keyCode::shift) ? ':' : ';';
		break;
	case keyCode::equals:
		k = Window::isKeyPress(keyCode::shift) ? '+' : '=';
		break;
	case keyCode::period:
		k = Window::isKeyPress(keyCode::shift) ? '>' : '.';
		break;
	case keyCode::comma:
		k = Window::isKeyPress(keyCode::shift) ? '<' : ',';
		break;
	case keyCode::slash:
		k = Window::isKeyPress(keyCode::shift) ? '?' : '/';
		break;
	case keyCode::backslash:
		k = Window::isKeyPress(keyCode::shift) ? '|' : '\\';
		break;
	case keyCode::quote:
		k = Window::isKeyPress(keyCode::shift) ? '\"' : '\'';
		break;
	case keyCode::tilda:
		k = Window::isKeyPress(keyCode::shift) ? '~' : '`';
		break;
	case (keyCode)'0':
		k = Window::isKeyPress(keyCode::shift) ? ')' : '0';
		break;
	case (keyCode)'1':
		k = Window::isKeyPress(keyCode::shift) ? '!' : '1';
		break;
	case (keyCode)'2':
		k = Window::isKeyPress(keyCode::shift) ? '@' : '2';
		break;
	case (keyCode)'3':
		k = Window::isKeyPress(keyCode::shift) ? '#' : '3';
		break;
	case (keyCode)'4':
		k = Window::isKeyPress(keyCode::shift) ? '$' : '4';
		break;
	case (keyCode)'5':
		k = Window::isKeyPress(keyCode::shift) ? '%' : '5';
		break;
	case (keyCode)'6':
		k = Window::isKeyPress(keyCode::shift) ? '^' : '6';
		break;
	case (keyCode)'7':
		k = Window::isKeyPress(keyCode::shift) ? '&' : '7';
		break;
	case (keyCode)'8':
		k = Window::isKeyPress(keyCode::shift) ? '*' : '8';
		break;
	case (keyCode)'9':
		k = Window::isKeyPress(keyCode::shift) ? '(' : '9';
		break;
	}
	if (c >= keyCode::n_0 && c <= keyCode::n_9)
		k = (int)c + '0';
	return k;
}
