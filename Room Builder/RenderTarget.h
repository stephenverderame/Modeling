#pragma once
#include "Observer.h"
class RenderTarget : public WindowObserver
{
protected:
	unsigned int width, height;
	unsigned int activeTexture;
	float r, g, b, a;
public:
	RenderTarget(unsigned int activeTexture = 0);
	virtual void bindForWriting() = 0;
	virtual void bindForReading() = 0;
	virtual void unBind() = 0;
	void getDimensions(unsigned int & w, unsigned int & h) { w = width; h = height; }
	void setClearColor(float r, float g, float b, float a);
};
class MultisampledRenderTarget : public RenderTarget
{
	unsigned int fbo, rbo, tex;
	unsigned int samples;
public:
	MultisampledRenderTarget(unsigned int width, unsigned int height, unsigned int samples = 4, unsigned int activeTexture = 0);
	~MultisampledRenderTarget();
	void bindForWriting() override;
	void bindForReading() override;
	void unBind() override;
	void notify(const message & cmd) override;
};
class RenderTarget2D : public RenderTarget
{
	unsigned int fbo, rbo, tex;
public:
	RenderTarget2D(unsigned int width, unsigned int height, unsigned int activeTexture = 0);
	~RenderTarget2D();
	void bindForWriting() override;
	void bindForReading() override;
	void unBind() override;
	void notify(const message & cmd) override;
};
class CubemapRenderTarget : public RenderTarget
{
	unsigned int fbo, rbo, tex;
public:
	CubemapRenderTarget(unsigned int width, unsigned int height, unsigned int activeTexture = 0);
	~CubemapRenderTarget();
	void bindForWriting() override;
	void bindForReading() override;
	void unBind() override;
	void notify(const message & cmd) override;
	void bindOutputFace(int target);
};
class OmnidirectionalShadowMap : public RenderTarget
{
	unsigned int fbo, depthMap;
public:
	OmnidirectionalShadowMap(unsigned int size, unsigned int activeTexture = 5);
	~OmnidirectionalShadowMap();
	void bindForWriting() override;
	void bindForReading() override;
	void unBind() override;
	void notify(const message & cmd) override;
};

