#include "Light.h"
#include "RenderTarget.h"

#include "ShaderManager.h"
#include "Shader.h"
#include <sstream>

struct lImpl
{
	std::unique_ptr<OmnidirectionalShadowMap> depthMap;
	glm::mat4 proj;
};

Light::Light()
{
	lpimpl = std::make_unique<lImpl>();
	lpimpl->depthMap = std::make_unique<OmnidirectionalShadowMap>(1024, 5);
	lpimpl->proj = glm::perspective(glm::radians(90.f), 1.f, 1.f, 30.f);
	requiredPasses |= RENDER_PASS_SHADOW;
	visible = false;
}

void Light::prepareDepthPass()
{
	glm::vec3 dirs[] = {
		{1.0, 0.0, 0.0},
		{-1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, -1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 0.0, -1.0}
	};
	glm::vec3 ups[] = {
		{0.0, 0.0, -1.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0}
	};
	glm::vec3 p = getPos();
	ShaderManager::getShader(shaderID::omniDepth)->setVec3("lightPos", p);
	ShaderManager::getShader(shaderID::omniDepth)->setFloat("farPlane", 30.f);
	for (int i = 0; i < 6; ++i)
	{
		glm::mat4 view = glm::lookAt(p, p + dirs[i], ups[(int)dirs[i].y + 1]);
		std::stringstream uniform;
		uniform << "lightSpaceTransforms[" << i << "]";
		ShaderManager::getShader(shaderID::omniDepth)->setMat4(uniform.str().c_str(), lpimpl->proj * view);
	}
	lpimpl->depthMap->bindForWriting();
}

Light::~Light() = default;

void Light::nvi_draw(int p)
{
	
}

void Light::finishDepthPass()
{
	lpimpl->depthMap->unBind();
	lpimpl->depthMap->bindForReading();
	ShaderManager::getShader(shaderID::basic)->setFloat("farPlane", 30.f);
	ShaderManager::getShader(shaderID::basic)->setVec3("lightPos", getPos());
}