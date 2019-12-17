#pragma once
#include "Observer.h"
class RenderTarget : public WindowObserver
{
public:
	virtual void bindForWriting() = 0;
	virtual void bindForReading() = 0;
	virtual void unBind() = 0;
};
class MultisampledRenderTarget : public RenderTarget
{
	unsigned int fbo, rbo, tex;
	unsigned int samples;
public:
	MultisampledRenderTarget(unsigned int width, unsigned int height, unsigned int samples = 4);
	~MultisampledRenderTarget();
	void bindForWriting() override;
	void bindForReading() override;
	void unBind() override;
	void notify(const message & cmd) override;
};

