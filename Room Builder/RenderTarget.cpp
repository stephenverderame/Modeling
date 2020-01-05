#include "RenderTarget.h"
#include <glad/glad.h>
#include <stdio.h>
#include "ShaderManager.h"
#include "Shader.h"

MultisampledRenderTarget::MultisampledRenderTarget(unsigned int width, unsigned int height, unsigned int samples, unsigned int texId) : samples(samples), RenderTarget(texId)
{
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + texId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_TRUE);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glViewport(0, 0, width, height);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Incomplete Init\n");
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
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
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, width, height);
}

void MultisampledRenderTarget::bindForReading()
{
	glActiveTexture(GL_TEXTURE0 + activeTexture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	ShaderManager::getShader(shaderID::compositor)->setInt("samples", samples);
	glActiveTexture(GL_TEXTURE0);
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
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Incomplete recreation\n");
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glActiveTexture(GL_TEXTURE0);
		
	}
}


RenderTarget2D::RenderTarget2D(unsigned int width, unsigned int height, unsigned int activeTexture) : RenderTarget(activeTexture)
{
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + activeTexture);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glViewport(0, 0, width, height);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Incomplete Init\n");
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	this->width = width;
	this->height = height;
}

RenderTarget2D::~RenderTarget2D()
{
	glDeleteRenderbuffers(1, &rbo);
	glDeleteTextures(1, &tex);
	glDeleteFramebuffers(1, &fbo);
}
void RenderTarget2D::bindForWriting()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderTarget2D::bindForReading()
{
	glActiveTexture(GL_TEXTURE0 + activeTexture);
	glBindTexture(GL_TEXTURE_2D, tex);
	glActiveTexture(GL_TEXTURE0);
}

void RenderTarget2D::unBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget2D::notify(const message & cmd)
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
		glActiveTexture(GL_TEXTURE0 + activeTexture);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Incomplete recreation\n");
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glActiveTexture(GL_TEXTURE0);

	}
}

RenderTarget::RenderTarget(unsigned int activeTexture) : activeTexture(activeTexture), r(0.2), g(0.2), b(0.2), a(0) {}
void RenderTarget::setClearColor(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

CubemapRenderTarget::CubemapRenderTarget(unsigned int width, unsigned int height, unsigned int activeTexture) : RenderTarget(activeTexture)
{
	glGetError();
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + activeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	for (int i = 0; i < 6; ++i)		
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex, 0);
//	for(int i = 0; i < 6; ++i)
//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Fbo fail!\n");
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	this->width = width;
	this->height = height;

}
void CubemapRenderTarget::bindOutputFace(int face)
{
//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, tex, 0);
//	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
//	glDrawBuffer(GL_COLOR_ATTACHMENT0 + (face - GL_TEXTURE_CUBE_MAP_POSITIVE_X));
}

void CubemapRenderTarget::bindForWriting()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void CubemapRenderTarget::bindForReading()
{
	glActiveTexture(GL_TEXTURE0 + activeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
}

void CubemapRenderTarget::notify(const message & m) {}

void CubemapRenderTarget::unBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

CubemapRenderTarget::~CubemapRenderTarget()
{
	glDeleteRenderbuffers(1, &rbo);
	glDeleteTextures(1, &tex);
	glDeleteFramebuffers(1, &fbo);
}

OmnidirectionalShadowMap::OmnidirectionalShadowMap(unsigned int size, unsigned int activeTexture) : RenderTarget(activeTexture)
{
	width = size;
	height = size;
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &depthMap);
	glActiveTexture(GL_TEXTURE0 + activeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OmnidirectionalShadowMap::~OmnidirectionalShadowMap()
{
	glDeleteTextures(1, &depthMap);
	glDeleteFramebuffers(1, &fbo);
}

void OmnidirectionalShadowMap::bindForWriting()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OmnidirectionalShadowMap::bindForReading()
{
	glActiveTexture(GL_TEXTURE0 + activeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
}

void OmnidirectionalShadowMap::unBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OmnidirectionalShadowMap::notify(const message & cmd)
{
}
