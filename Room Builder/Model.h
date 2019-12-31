#pragma once
#include "Object.h"
class Model : public Object
{
	struct mesh;
	struct mImpl;
	std::unique_ptr<mImpl> mPimpl;
protected:
	void nvi_draw(renderPass p) override;
public:
	Model(const char * filename);
	~Model();
private:
	mesh processMesh(void * aiMesh, const void * aiScene);
	void processNode(void * aiNode, const void * aiScene);
};

