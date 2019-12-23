#include "ShaderManager.h"
#include <vector>
#include "Shader.h"
struct smImpl
{
	std::vector<std::unique_ptr<Shader>> shaders;
};
unsigned int ShaderManager::activeShader = ~0;
const ShaderPasskey ShaderManager::key = ShaderPasskey();
ShaderManager::ShaderManager(const ShaderPasskey & key)
{
	pimpl = std::make_unique<smImpl>();
	pimpl->shaders.push_back(std::make_unique<Shader>(key, "svert.glsl", "sfrag.glsl"));
	pimpl->shaders.push_back(std::make_unique<Shader>(key, "invert.glsl", "sfrag.glsl"));
	pimpl->shaders.push_back(std::make_unique<Shader>(key, "compositeVert.glsl", "compositeFrag.glsl"));
	pimpl->shaders.push_back(std::make_unique<Shader>(key, "guiVert.glsl", "guiFrag.glsl"));
	//	pimpl->shaders.emplace_back("invert.glsl", "sfrag.glsl");
}
ShaderManager::~ShaderManager()
{

}
std::unique_ptr<ShaderManager> ShaderManager::singleton = nullptr;
void ShaderManager::useShader(unsigned int program)
{
	if (activeShader != program) {
		glUseProgram(program);
		activeShader = program;
	}
}
void ShaderManager::useShader(shaderID id)
{
	unsigned int program = singleton->pimpl->shaders[(int)id]->getProgram(ShaderManager::key);
	if (activeShader != program) {
		glUseProgram(program);
		activeShader = program;
	}
}
/*
void ShaderManager::updateProjections(const glm::mat4 & proj)
{
	singleton->pimpl->shaders[(int)shaderID::basic]->setMat4("projection", proj);
	singleton->pimpl->shaders[(int)shaderID::instance]->setMat4("projection", proj);
}
*/
std::unique_ptr<Shader>& ShaderManager::getShader(shaderID id)
{
	if (singleton == nullptr)
		singleton = std::make_unique<ShaderManager>(key);
	useShader(singleton->pimpl->shaders[(int)id]->getProgram(key));
	return singleton->pimpl->shaders[(int)id];
}