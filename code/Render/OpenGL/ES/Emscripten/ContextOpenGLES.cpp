#include <cstring>
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/ES/ExtensionsGLES.h"
#include "Render/OpenGL/ES/Emscripten/ContextOpenGLES.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_maxMatchConfigs = 64;

typedef RefArray< ContextOpenGLES > context_stack_t;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGLES", ContextOpenGLES, Object)

ThreadLocal ContextOpenGLES::ms_contextStack;

Ref< ContextOpenGLES > ContextOpenGLES::createContext(const SystemApplication& sysapp, const RenderViewDefaultDesc& desc)
{
	Ref< ContextOpenGLES > context = new ContextOpenGLES();

	context->m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (context->m_display == EGL_NO_DISPLAY)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES failed; unable to get EGL display (" << getEGLErrorString(error) << L")" << Endl;
		return nullptr;
	}

	if (!eglInitialize(context->m_display, 0, 0))
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES failed; unable to initialize EGL (" << getEGLErrorString(error) << L")" << Endl;
		return nullptr;
	}

	EGLConfig matchingConfigs[c_maxMatchConfigs];
	EGLint numMatchingConfigs = 0;

	if (desc.multiSample > 1)
	{
		const EGLint configAttribsWithMSAA[] =
		{
			EGL_LEVEL, 0,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
			EGL_DEPTH_SIZE, desc.depthBits,
			EGL_STENCIL_SIZE, desc.stencilBits,
			EGL_SAMPLES, (EGLint)desc.multiSample,
			EGL_NONE
		};

		EGLBoolean success = eglChooseConfig(
			context->m_display,
			configAttribsWithMSAA,
			matchingConfigs,
			c_maxMatchConfigs,
			&numMatchingConfigs
		);
		if (!success || numMatchingConfigs == 0)
			log::warning << L"No matching MSAA configurations found; MSAA disabled." << Endl;
	}

	if (numMatchingConfigs == 0)
	{
		const EGLint configAttribs[] =
		{
			EGL_LEVEL, 0,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
			EGL_DEPTH_SIZE, desc.depthBits,
			EGL_STENCIL_SIZE, desc.stencilBits,
			EGL_NONE
		};

		EGLBoolean success = eglChooseConfig(
			context->m_display,
			configAttribs,
			matchingConfigs,
			c_maxMatchConfigs,
			&numMatchingConfigs
		);
		if (!success)
		{
			EGLint error = eglGetError();
			log::error << L"Create OpenGL ES failed; unable to choose EGL config (" << getEGLErrorString(error) << L")." << Endl;
			return nullptr;
		}
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES failed; no matching configurations." << Endl;
		return nullptr;
	}

	context->m_config = matchingConfigs[0];

	context->m_surface = eglCreateWindowSurface(context->m_display, context->m_config, 0, 0);
	if (context->m_surface == EGL_NO_SURFACE)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES context failed; unable to create EGL surface (" << getEGLErrorString(error) << L")" << Endl;
		return nullptr;
	}

	eglBindAPI(EGL_OPENGL_ES_API);

	const EGLint contextAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};

	context->m_context = eglCreateContext(
		context->m_display,
		context->m_config,
		EGL_NO_CONTEXT,
		contextAttribs
	);
	if (context->m_context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES context failed (1); unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return nullptr;
	}

	if (!context->enter())
		return nullptr;
	initializeExtensions();
	context->leave();

	log::info << L"OpenGL ES render context created successfully" << Endl;
	return context;
}

Ref< ContextOpenGLES > ContextOpenGLES::createContext(const SystemApplication& sysapp, const RenderViewEmbeddedDesc& desc)
{
	return nullptr;
}

bool ContextOpenGLES::reset(int32_t width, int32_t height)
{
	return true;
}

bool ContextOpenGLES::enter()
{
	if (!m_lock.wait())
		return false;

	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	if (!stack)
	{
		stack = new context_stack_t();
		ms_contextStack.set(stack);
	}

	if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context))
	{
		EGLint error = eglGetError();
		log::error << L"Enter OpenGL ES context failed; " << getEGLErrorString(error) << Endl;
		m_lock.release();
		return false;
	}

	stack->push_back(this);
	return true;
}

void ContextOpenGLES::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());

	T_ASSERT (stack);
	T_ASSERT (!stack->empty());
	T_ASSERT (stack->back() == this);

	stack->pop_back();

	if (!stack->empty())
		eglMakeCurrent(m_display, stack->back()->m_surface, stack->back()->m_surface, stack->back()->m_context);
	else
		eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	m_lock.release();
}

void ContextOpenGLES::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextOpenGLES::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (!m_deleteResources.empty())
	{
		enter();
		for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
			(*i)->deleteResource();
		m_deleteResources.resize(0);
		leave();
	}
}

GLuint ContextOpenGLES::createShaderObject(const char* shader, GLenum shaderType)
{
	Adler32 adler;
	adler.begin();
	adler.feed(shader, strlen(shader));
	adler.end();

	uint32_t hash = adler.get();

	std::map< uint32_t, GLuint >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLuint shaderObject = glCreateShader(shaderType);
	if (shaderObject == 0)
	{
		log::error << L"Failed to compile GLSL shader; glCreateShader returned 0" << Endl;
		return 0;
	}

	T_OGL_SAFE(glShaderSource(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShader(shaderObject));

	char errorBuf[32000];
	GLint status;

	T_OGL_SAFE(glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status));
	if (status == 0)
	{
		T_OGL_SAFE(glGetShaderInfoLog(shaderObject, sizeof(errorBuf), 0, errorBuf));
		log::error << L"Failed to compile GLSL shader:" << Endl;
		log::error << mbstows(errorBuf) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, mbstows(shader));
		return 0;
	}

	m_shaderObjects.insert(std::make_pair(hash, shaderObject));
	return shaderObject;
}

int32_t ContextOpenGLES::getWidth() const
{
	EGLint width;
	eglQuerySurface(m_display, m_surface, EGL_WIDTH, &width);
	return width;
}

int32_t ContextOpenGLES::getHeight() const
{
	EGLint height;
	eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &height);
	return height;
}

void ContextOpenGLES::swapBuffers()
{
	eglSwapBuffers(m_display, m_surface);
}

Semaphore& ContextOpenGLES::lock()
{
	return m_lock;
}

void ContextOpenGLES::bindPrimary()
{
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	T_OGL_SAFE(glViewport(
		0,
		0,
		getWidth(),
		getHeight()
	));

	if (!m_primaryDepth)
	{
		T_OGL_SAFE(glGenRenderbuffers(1, &m_primaryDepth));
		T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, m_primaryDepth));
		T_OGL_SAFE(glRenderbufferStorage(
			GL_RENDERBUFFER,
			m_primaryDepthFormat,
			getWidth(),
			getHeight()
		));
	}
}

GLuint ContextOpenGLES::getPrimaryDepth() const
{
	return m_primaryDepth;
}

ContextOpenGLES::ContextOpenGLES()
:	m_primaryDepthFormat(GL_DEPTH_COMPONENT16)
,	m_primaryDepth(0)
{
}

	}
}
