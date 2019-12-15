#pragma once
#include <memory>
struct message 
{
	struct HWND__ * hwnd;
	unsigned int msg;
#ifdef _WIN64
	unsinged long long wparam;
	long long lparam;
#else
	unsigned int wparam;
	long lparam;
#endif
};
class WindowObserver
{
public:
	virtual void notify(const message & msg) = 0;
};

enum class msg
{
	cm_zoom, //[int direction]
	cm_pan, //[int x, int y]
	cm_rotate, //[int x, int y]
	cm_wndSize, //[int w, int h]
	cm_translateObj, //[int x, int y, int axis]
	cm_msgs,
	click, //[int x, int y]
	sn_select, //[vec4 * pos]
	sn_translateObj, //[vec3 * deltaPos]
	sn_msgs,
	nill = INT_MAX
};
struct command
{
	msg cmd;
	void * args[4];

};
class Observer
{
public:
	virtual void notify(const command & cmd) = 0;
	virtual bool isInterested(msg m) = 0;
};