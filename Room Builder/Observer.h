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
	cm_msgs,
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