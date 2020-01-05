#pragma once
#include "Object.h"
struct lImpl;
class Light : public Object
{
	std::unique_ptr<lImpl> lpimpl;
public:
	Light();
	void prepareDepthPass();
	void finishDepthPass();
	~Light();
protected:
	void nvi_draw(int p) override;
};

