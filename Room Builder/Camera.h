#pragma once
#include <memory>
#include "Observer.h"
struct camImpl;
class Camera : public Observer
{
	std::unique_ptr<camImpl> pimpl;
private:
	void updateCam();
public:
	Camera();
	~Camera();
	void setViewMatrix(enum class shaderID sid);
	void pan(float dx, float dy);
	void zoom(float dz);
	void rotate(float dx, float dy);
	void notify(const command & msg) override;
	bool isInterested(msg m) override;
};

