/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetDepthOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ResourceContextOpenGL::IDeleteCallback
{
	GLuint m_textureName;

	DeleteTextureCallback(GLuint textureName)
	:	m_textureName(textureName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteTextures(1, &m_textureName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthOpenGL", RenderTargetDepthOpenGL, ISimpleTexture)

RenderTargetDepthOpenGL::RenderTargetDepthOpenGL(ResourceContextOpenGL* resourceContext, GLuint depthTexture, int32_t width, int32_t height)
:	m_resourceContext(resourceContext)
,	m_depthTexture(depthTexture)
,	m_width(width)
,	m_height(height)
{
}

RenderTargetDepthOpenGL::~RenderTargetDepthOpenGL()
{
	destroy();
}

void RenderTargetDepthOpenGL::destroy()
{
	if (m_depthTexture)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteTextureCallback(m_depthTexture));
		m_depthTexture = 0;
	}
}

ITexture* RenderTargetDepthOpenGL::resolve()
{
	return this;
}

int RenderTargetDepthOpenGL::getWidth() const
{
	return m_width;
}

int RenderTargetDepthOpenGL::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthOpenGL::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetDepthOpenGL::unlock(int level)
{
}

void* RenderTargetDepthOpenGL::getInternalHandle()
{
	return (void*)m_depthTexture;
}

void RenderTargetDepthOpenGL::bindTexture() const
{
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_depthTexture));
}

void RenderTargetDepthOpenGL::bindSize(GLint locationSize) const
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

bool RenderTargetDepthOpenGL::haveMips() const
{
	return false;
}

	}
}
