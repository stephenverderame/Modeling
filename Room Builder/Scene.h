#pragma once
#include <memory>
#include "Observer.h"
struct sImpl;
class Scene : public Observer
{
	std::unique_ptr<sImpl> pimpl;
public:
	Scene();
	~Scene();
	void addObject(std::shared_ptr<class Object> & obj);
	void addObject(std::shared_ptr<class Object> && obj);
	void renderScene();
	void notify(const command & cmd) override;
	bool isInterested(msg m) override;

};

