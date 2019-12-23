#include "RenderTarget.h"
#include <glad/glad.h>
#include <stdio.h>
#include "ShaderManager.h"
#include "Shader.h"

MultisampledRenderTarget::MultisampledRenderTarget(unsigned int width, unsigned int height, unsigned int samples) : samples(samples)
{
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_TRUE);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glViewport(0, 0, width, height);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Incomplete Init\n");
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	this->width = width;
	this->height = height;
}

MultisampledRenderTarget::~MultisampledRenderTarget()
{
	glDeleteRenderbuffers(1, &rbo);
	glDeleteTextures(1, &tex);
	glDeleteFramebuffers(1, &fbo);
}

void MultisampledRenderTarget::bindForWriting()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2, 0.2, 0.2, 0.0);
}

void MultisampledRenderTarget::bindForReading()
{
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	ShaderManager::getShader(shaderID::compositor)->setInt("samples", samples);
}

void MultisampledRenderTarget::unBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MultisampledRenderTarget::notify(const message & cmd)
{
	if (cmd.msg == WM_SIZE) {
		int width = LOWORD(cmd.lparam);
		int height = HIWORD(cmd.lparam);
		this->width = width;
		this->height = height;
		glDeleteTextures(1, &tex);
		glDeleteRenderbuffers(1, &rbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glGenTextures(1, &tex);
		glGenRenderbuffers(1, &rbo);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_TRUE);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Incomplete recreation\n");
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		
	}
}
