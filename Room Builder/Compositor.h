#pragma once
#include <memory>
#include "Observer.h"
struct cmImpl;
class Compositor : public WindowObserver
{
	std::unique_ptr<cmImpl> pimpl;
public:
	Compositor(unsigned int width, unsigned int height);
	~Compositor();
	//whatever is composed last will be most frontal
	void compose(class Composable & comp);
	void notify(const message & msg) override;
	void renderFinal();
};

