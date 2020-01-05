#pragma once
#include <memory>
#include "Observer.h"
#include "Composable.h"
#include "Object.h"
struct sImpl;
class Scene : public Composable
{
protected:
	std::unique_ptr<sImpl> pimpl;
protected:
	virtual void renderScenePreconditions() {};
	virtual void renderScenePostconditions() {};
	virtual void nvi_addObject(std::shared_ptr<class Object> obj) {};
	virtual void renderObjPreconditions(std::shared_ptr<class Object> obj) {};
public:
	Scene(class RenderTarget & rt);
	virtual ~Scene();
	void addObject(std::shared_ptr<class Object> obj);
	void renderScene(int p = RENDER_PASS_STANDARD);
	void compose() override;

};
struct msImpl;
class MainScene : public Scene, public Observer
{
	std::unique_ptr<msImpl> mpimpl;
public:
	static const float BASE_UNIT;
public:
	using Scene::addObject;
	void notify(const command & cmd) override;
	bool isInterested(msg m) override;
	void handlePasses(class Camera & cam);
	void addObject(std::shared_ptr<class Light> obj);
	MainScene(class RenderTarget & rt);
	~MainScene();
protected:
	void renderScenePreconditions() override;
	void nvi_addObject(std::shared_ptr<class Object> obj) override;
	void renderObjPreconditions(std::shared_ptr<class Object> obj) override;
};

struct gImpl;
class GuiScene : public Scene, public Observer
{
	std::unique_ptr<gImpl> gpimpl;
	std::shared_ptr<class Font> font;
public:
	void clickCallback(int id, const char * name);
	void addCallback();
public:
	void notify(const command & cmd) override;
	bool isInterested(msg m) override;
	GuiScene(class RenderTarget & rt, std::shared_ptr<class Font> f, class Scene & sn);
	~GuiScene();
protected:
	void renderScenePreconditions() override;
	void renderScenePostconditions() override;
};

