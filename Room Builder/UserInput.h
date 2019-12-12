#pragma once
#include "Observer.h"
#include <memory>
struct uiImpl;
class UserInput : public WindowObserver
{
	std::unique_ptr<uiImpl> pimpl;
private:
	void notifyInterestedObservers(const command & cmd);
public:
	UserInput();
	~UserInput();
	void notify(const message & msg) override;
	void attach(Observer & observer);
};

