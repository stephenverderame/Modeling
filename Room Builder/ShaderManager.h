#pragma once
#include <memory>
struct smImpl;
enum class shaderID
{
	basic, instance
};
class Shader;
class ShaderPasskey
{
	friend class ShaderManager;
private:
	ShaderPasskey() = default;
public:
	~ShaderPasskey() = default;
};
class ShaderManager
{
private:
	const static ShaderPasskey key;
	static unsigned int activeShader;
	static std::unique_ptr<ShaderManager> singleton;
	std::unique_ptr<smImpl> pimpl;
public:
	ShaderManager(const ShaderPasskey & key);
public:
	static void useShader(unsigned int program);
	static std::unique_ptr<Shader> & getShader(shaderID id);
	~ShaderManager();
};