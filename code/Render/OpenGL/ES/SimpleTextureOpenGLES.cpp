#include <algorithm>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Render/OpenGL/ES/Platform.h"
#include "Render/OpenGL/ES/SimpleTextureOpenGLES.h"
#include "Render/OpenGL/ES/UtilitiesOpenGLES.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES/Android/ContextOpenGLES.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES/iOS/ContextOpenGLES.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES/Emscripten/ContextOpenGLES.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES/Win32/ContextOpenGLES.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/OpenGL/ES/Linux/ContextOpenGLES.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ContextOpenGLES::IDeleteCallback
{
	GLuint m_textureName;

	DeleteTextureCallback(GLuint textureName)
	:	m_textureName(textureName)
	{
	}

	virtual ~DeleteTextureCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteTextures(1, &m_textureName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureOpenGLES", SimpleTextureOpenGLES, ISimpleTexture)

SimpleTextureOpenGLES::SimpleTextureOpenGLES(ContextOpenGLES* context)
:	m_context(context)
,	m_textureName(0)
,	m_pot(false)
,	m_dirty(-1)
,	m_width(0)
,	m_height(0)
,	m_pixelSize(0)
,	m_mipCount(0)
{
	std::memset(&m_shadowState, 0, sizeof(m_shadowState));
}

SimpleTextureOpenGLES::~SimpleTextureOpenGLES()
{
	destroy();
}

bool SimpleTextureOpenGLES::create(const SimpleTextureCreateDesc& desc)
{
	m_pot = isLog2(desc.width) && isLog2(desc.height);
	m_width = desc.width;
	m_height = desc.height;

	if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
	{
		log::error << L"Unable to create simple texture; unsupported format \"L" << getTextureFormatName(desc.format) << L"\"." << Endl;
		return false;
	}

	T_OGL_SAFE(glGenTextures(1, &m_textureName));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));
	T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

	// Set default parameters as its might help driver.
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	if (m_pot)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}
	else
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	// Allocate data buffer.
	uint32_t texturePitch = getTextureMipPitch(desc.format, desc.width, desc.height);
	if (desc.immutable)
	{
		for (int i = 0; i < desc.mipCount; ++i)
		{
			uint32_t width = std::max(m_width >> i, 1);
			uint32_t height = std::max(m_height >> i, 1);

			if (
				(desc.format >= TfDXT1 && desc.format <= TfDXT5) ||
				(desc.format >= TfPVRTC1 && desc.format <= TfPVRTC4) ||
				desc.format == TfETC1
			)
			{
				uint32_t mipSize = getTextureMipPitch(desc.format, width, height);
				T_OGL_SAFE(glCompressedTexImage2D(
					GL_TEXTURE_2D,
					i,
					m_components,
					width,
					height,
					0,
					mipSize,
					desc.initialData[i].data
				));
			}
			else
			{
				T_OGL_SAFE(glTexImage2D(
					GL_TEXTURE_2D,
					i,
					m_components,
					width,
					height,
					0,
					m_format,
					m_type,
					desc.initialData[i].data
				));
			}
		}
	}
	else
	{
		m_data.resize(texturePitch, 0);

		for (int i = 0; i < desc.mipCount; ++i)
		{
			uint32_t width = std::max(m_width >> i, 1);
			uint32_t height = std::max(m_height >> i, 1);

			T_OGL_SAFE(glTexImage2D(
				GL_TEXTURE_2D,
				i,
				m_components,
				width,
				height,
				0,
				m_format,
				m_type,
				desc.initialData[i].data ? desc.initialData[i].data : m_data.c_ptr()
			));
		}
	}

	m_mipCount = desc.mipCount;
	return true;
}

void SimpleTextureOpenGLES::destroy()
{
	if (m_textureName)
	{
		if (m_context)
			m_context->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

ITexture* SimpleTextureOpenGLES::resolve()
{
	return this;
}

int32_t SimpleTextureOpenGLES::getMips() const
{
	return m_mipCount;
}

int32_t SimpleTextureOpenGLES::getWidth() const
{
	return m_width;
}

int32_t SimpleTextureOpenGLES::getHeight() const
{
	return m_height;
}

bool SimpleTextureOpenGLES::lock(int32_t level, Lock& lock)
{
	if (m_data.empty() || level >= m_mipCount)
		return false;

	lock.pitch = std::max(m_width >> level, 1) * m_pixelSize;
	lock.bits = &m_data[0];
	return true;
}

void SimpleTextureOpenGLES::unlock(int32_t level)
{
	m_dirty = level;
}

void* SimpleTextureOpenGLES::getInternalHandle()
{
	return (void*)m_textureName;
}

void SimpleTextureOpenGLES::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));

	if (m_dirty >= 0)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glTexSubImage2D(
			GL_TEXTURE_2D,
			m_dirty,
			0,
			0,
			std::max(m_width >> m_dirty, 1),
			std::max(m_height >> m_dirty, 1),
			m_format,
			m_type,
			m_data.c_ptr()
		));
		m_dirty = -1;
	}

	GLenum minFilter = GL_NEAREST;
	if (m_mipCount > 1)
		minFilter = samplerState.minFilter;

	if (m_shadowState.minFilter != minFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
		m_shadowState.minFilter = minFilter;
	}

	if (m_shadowState.magFilter != samplerState.magFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		m_shadowState.magFilter = samplerState.magFilter;
	}

	if (m_pot)
	{
		if (m_shadowState.wrapS != samplerState.wrapS)
		{
			T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, samplerState.wrapS));
			m_shadowState.wrapS = samplerState.wrapS;
		}

		if (m_shadowState.wrapT != samplerState.wrapT)
		{
			T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, samplerState.wrapT));
			m_shadowState.wrapT = samplerState.wrapT;
		}
	}

	T_OGL_SAFE(glUniform1i(locationTexture, unit));
}

void SimpleTextureOpenGLES::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(0), GLfloat(0)));
}

	}
}
