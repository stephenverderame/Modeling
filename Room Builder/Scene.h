#pragma once
#include <memory>
#include "Observer.h"
#include "Composable.h"
struct sImpl;
class Scene : public Composable
{
protected:
	std::unique_ptr<sImpl> pimpl;
protected:
	virtual void renderScenePreconditions() {};
	virtual void renderScenePostconditions() {};
	virtual void nvi_addObject(std::shared_ptr<class Object> obj) {};
public:
	Scene(class RenderTarget & rt);
	virtual ~Scene();
	void addObject(std::shared_ptr<class Object> & obj);
	void addObject(std::shared_ptr<class Object> && obj);
	void renderScene();
	void compose() override;

};
struct msImpl;
class MainScene : public Scene, public Observer
{
	std::unique_ptr<msImpl> mpimpl;
public:
	static const float BASE_UNIT;
public:
	void notify(const command & cmd) override;
	bool isInterested(msg m) override;
	MainScene(class RenderTarget & rt);
	~MainScene();
protected:
	void renderScenePreconditions() override;
	void nvi_addObject(std::shared_ptr<class Object> obj) override;
};

