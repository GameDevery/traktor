#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/BlitHelper.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/IndexBufferIAR.h"
#include "Render/OpenGL/Std/IndexBufferIBO.h"
#include "Render/OpenGL/Std/ProgramCompilerOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/VertexBufferVAR.h"
#include "Render/OpenGL/Std/VertexBufferDynamicVBO.h"
#include "Render/OpenGL/Std/VertexBufferStaticVBO.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"

#if defined(_WIN32)
#	include "Render/OpenGL/Std/Win32/Window.h"
#elif defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#	include "Render/OpenGL/Std/OsX/CGLWindow.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemOpenGL", 0, RenderSystemOpenGL, IRenderSystem)

RenderSystemOpenGL::RenderSystemOpenGL()
#if defined(__APPLE__)
:	m_windowHandle(0)
,	m_maxAnisotrophy(1.0f)
#elif defined(__LINUX__)
:	m_display(0)
,	m_maxAnisotrophy(1.0f)
#else
:	m_maxAnisotrophy(1.0f)
#endif
{
}

bool RenderSystemOpenGL::create(const RenderSystemCreateDesc& desc)
{
#if defined(_WIN32)

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	m_windowShared = new Window();
	if (!m_windowShared->create())
		return false;

	HDC hSharedDC = GetDC(*m_windowShared);
	if (!hSharedDC)
		return false;

	int pixelFormat = ChoosePixelFormat(hSharedDC, &pfd);
	if (!pixelFormat)
		return false;

	if (!SetPixelFormat(hSharedDC, pixelFormat, &pfd))
		return false;


	// Create a dummy, old, context to load all extensions.
	HGLRC hDummyRC = wglCreateContext(hSharedDC);
	T_ASSERT (hDummyRC);

	wglMakeCurrent(hSharedDC, hDummyRC);

	if (!opengl_initialize_extensions())
		return false;

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hDummyRC);


	// Finally create new type of context.
	const GLint attribs[] =
	{
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0, 0
	};

	HGLRC hSharedRC = wglCreateContextAttribsARB(hSharedDC, NULL, attribs);
	T_ASSERT (hSharedRC);

	m_resourceContext = new ContextOpenGL(*m_windowShared, hSharedDC, hSharedRC);
	m_resourceContext->enter();

#elif defined(__APPLE__)

	void* resourceContext = cglwCreateContext(0, 0, 0, 0, 0);
	if (!resourceContext)
		return false;

	m_resourceContext = new ContextOpenGL(resourceContext);
	m_resourceContext->enter();

	if (!opengl_initialize_extensions())
		return false;

#elif defined(__LINUX__)

	XInitThreads();

	m_display = XOpenDisplay(0);
	if (!m_display)
	{
		log::error << L"Unable to create OpenGL renderer; Failed to open X display" << Endl;
		return 0;
	}

	m_windowShared = new Window(m_display);
	if (!m_windowShared->create(16, 16))
	{
		log::error << L"Unable to create OpenGL renderer; Failed to create resource window" << Endl;
		return 0;
	}

	int screen = DefaultScreen(m_windowShared->getDisplay());

	static int attribs[] =
	{
		GLX_RGBA,
		None
	};
	XVisualInfo* visual = glXChooseVisual(m_windowShared->getDisplay(), screen, attribs);
	if (!visual)
	{
		log::error << L"Unable to create OpenGL renderer; No visual found" << Endl;
		return false;
	}

	GLXContext resourceContext = glXCreateContext(m_windowShared->getDisplay(), visual, NULL, GL_TRUE);
	if (!resourceContext)
	{
		log::error << L"Unable to create OpenGL renderer; glXCreateContext failed" << Endl;
		return false;
	}

	m_resourceContext = new ContextOpenGL(m_windowShared->getDisplay(), m_windowShared->getWindow(), resourceContext);
	m_resourceContext->enter();

	if (!opengl_initialize_extensions())
	{
		log::error << L"Unable to create OpenGL renderer; Failed to initialize OpenGL extensions" << Endl;
		return false;
	}

#endif

	m_blitHelper = new BlitHelper();
	m_blitHelper->create();

	m_resourceContext->leave();

	m_maxAnisotrophy = (GLfloat)desc.maxAnisotropy;
	return true;
}

void RenderSystemOpenGL::destroy()
{
	if (m_resourceContext)
	{
		// Clean pending resources; don't want to leak resources.
		m_resourceContext->enter();
		m_resourceContext->deleteResources();
		m_resourceContext->leave();

		// Destroy resource context.
		m_resourceContext->destroy();
		m_resourceContext = 0;
	}

#if defined(_WIN32)

	m_windowShared = 0;

#elif defined(__APPLE__)

	if (m_windowHandle)
	{
		cglwDestroyWindow(m_windowHandle);
		m_windowHandle = 0;
	}

#elif defined(__LINUX__)

	m_windowShared = 0;

#endif
}

uint32_t RenderSystemOpenGL::getDisplayModeCount() const
{
#if defined(_WIN32)

	uint32_t count = 0;

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	while (EnumDisplaySettings(NULL, count, &dmgl))
		++count;

	return count;

#elif defined(__APPLE__)

	return cglwGetDisplayModeCount();

#else
	return 0;
#endif
}

DisplayMode RenderSystemOpenGL::getDisplayMode(uint32_t index) const
{
#if defined(_WIN32)

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, index, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;

#elif defined(__APPLE__)

	DisplayMode dm;
	cglwGetDisplayMode(index, dm);
	return dm;

#else
	return DisplayMode();
#endif
}

DisplayMode RenderSystemOpenGL::getCurrentDisplayMode() const
{
#if defined(_WIN32)

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;

#elif defined(__APPLE__)

	DisplayMode dm;
	cglwGetCurrentDisplayMode(dm);
	return dm;

#elif defined(__LINUX__)

	DisplayMode dm;
	dm.width = 1280;
	dm.height = 720;
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;

#else
	return DisplayMode();
#endif
}

float RenderSystemOpenGL::getDisplayAspectRatio() const
{
	return 0.0f;
}

Ref< IRenderView > RenderSystemOpenGL::createRenderView(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32)

	if (m_window)
		return 0;

	m_window = new Window();
	if (!m_window->create())
	{
		log::error << L"createRenderView failed; unable to create window" << Endl;
		return 0;
	}

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");

	if (desc.fullscreen)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

	if (desc.fullscreen)
	{
		DEVMODE dmgl;
		std::memset(&dmgl, 0, sizeof(dmgl));
		dmgl.dmSize = sizeof(dmgl);

		for (UINT count = 0; EnumDisplaySettings(NULL, count, &dmgl); ++count)
		{
			if (
				dmgl.dmPelsWidth == desc.displayMode.width &&
				dmgl.dmPelsHeight == desc.displayMode.height
			)
			{
				if (desc.displayMode.colorBits != 0 && dmgl.dmBitsPerPel != desc.displayMode.colorBits)
					continue;
				if (desc.displayMode.colorBits == 0 && dmgl.dmBitsPerPel < 24)
					continue;
				if (desc.displayMode.refreshRate != 0 && dmgl.dmDisplayFrequency != desc.displayMode.refreshRate)
					continue;

				if (ChangeDisplaySettings(&dmgl, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					log::error << L"createRenderView failed; unable to change display settings" << Endl;
					return 0;
				}

				break;
			}
		}
	}

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)desc.depthBits,
		(BYTE)desc.stencilBits,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC hDC = GetDC(*m_window);
	if (!hDC)
	{
		log::error << L"createRenderView failed; unable to get device context" << Endl;
		return 0;
	}

	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat)
	{
		log::error << L"createRenderView failed; unable to choose pixel format" << Endl;
		return 0;
	}

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
	{
		log::error << L"createRenderView failed; unable to set pixel format" << Endl;
		return 0;
	}

	const GLint attribs[] =
	{
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0, 0
	};

	HGLRC hRC = wglCreateContextAttribsARB(hDC, NULL, attribs);
	if (!hRC)
	{
		log::error << L"createRenderView failed; unable to create WGL context" << Endl;
		return 0;
	}

	Ref< ContextOpenGL > context = new ContextOpenGL(*m_window, hDC, hRC);
	m_resourceContext->share(context);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_window, context, m_resourceContext, m_blitHelper);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#elif defined(__APPLE__)

	m_windowHandle = cglwCreateWindow(
		!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer",
		desc.displayMode,
		desc.fullscreen
	);
	if (!m_windowHandle)
	{
		log::error << L"createRenderView failed; unable to create window" << Endl;
		return 0;
	}

	void* viewHandle = cglwGetWindowView(m_windowHandle);
	T_ASSERT (viewHandle);

	void* glcontext = cglwCreateContext(
		viewHandle,
		m_resourceContext->getGLContext(),
		desc.depthBits,
		desc.stencilBits,
		0
	);
	if (!glcontext)
	{
		log::error << L"createRenderView failed; unable to create GL context" << Endl;
		return 0;
	}

	Ref< ContextOpenGL > context = new ContextOpenGL(glcontext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_windowHandle, context, m_resourceContext, m_blitHelper);
	if (renderView->createPrimaryTarget())
		return renderView;

	log::error << L"createRenderView failed; unable to create primary render target" << Endl;

	context->destroy();
	context = 0;

#elif defined(__LINUX__)

    if (m_window)
        return 0;

	m_window = new Window(m_display);
	if (!m_window->create(desc.displayMode.width, desc.displayMode.height))
	{
		log::error << L"createRenderView failed; unable to create window" << Endl;
		return 0;
	}

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");

/*
	if (desc.fullscreen)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);
*/

	Display* display = m_window->getDisplay();
	T_ASSERT (display);

	int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, desc.depthBits, None };
	XVisualInfo* visual = glXChooseVisual(display, DefaultScreen(display), attribs);
	if (!visual)
		return 0;

	GLXContext glcontext = glXCreateContext(display, visual, m_resourceContext->getGLXContext(), GL_TRUE);
	if (!glcontext)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL(display, (GLXDrawable)m_window->getWindow(), glcontext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_window, context, m_resourceContext, m_blitHelper);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#endif

	return 0;
}

Ref< IRenderView > RenderSystemOpenGL::createRenderView(const RenderViewEmbeddedDesc& desc)
{
#if defined(_WIN32)

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)desc.depthBits,
		(BYTE)desc.stencilBits,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC hDC = GetDC((HWND)desc.nativeWindowHandle);
	if (!hDC)
		return 0;

	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat)
		return 0;

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
		return 0;

	HGLRC hRC = wglCreateContext(hDC);
	if (!hRC)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL((HWND)desc.nativeWindowHandle, hDC, hRC);
	m_resourceContext->share(context);

	context->enter();

	if (!opengl_initialize_extensions())
		return 0;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext, m_blitHelper);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#elif defined(__APPLE__)

	void* glcontext = cglwCreateContext(
		desc.nativeWindowHandle,
		m_resourceContext->getGLContext(),
		desc.depthBits,
		desc.stencilBits,
		0
	);
	if (!glcontext)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL(glcontext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext, m_blitHelper);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#elif defined(__LINUX__)

	Display* display = XOpenDisplay(0);
	if (!display)
		return 0;

	int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, desc.depthBits, None };
	XVisualInfo* visual = glXChooseVisual(display, DefaultScreen(display), attribs);
	if (!visual)
		return 0;

	GLXContext resourceContext = glXCreateContext(display, visual, NULL, GL_TRUE);
	if (!resourceContext)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL(display, (GLXDrawable)desc.nativeWindowHandle, resourceContext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext, m_blitHelper);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#endif

	return 0;
}

Ref< VertexBuffer > RenderSystemOpenGL::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

#if defined(_WIN32)

	if (glGenBuffersARB)
	{
		if (dynamic)
			return new VertexBufferDynamicVBO(m_resourceContext, vertexElements, bufferSize);
		else
			return new VertexBufferStaticVBO(m_resourceContext, vertexElements, bufferSize);
	}
	else
		return new VertexBufferVAR(m_resourceContext, vertexElements, bufferSize, dynamic);

#elif defined(__APPLE__) || defined(__LINUX__)

	if (opengl_have_extension(E_GL_ARB_vertex_buffer_object))
	{
		if (dynamic)
			return new VertexBufferDynamicVBO(m_resourceContext, vertexElements, bufferSize);
		else
			return new VertexBufferStaticVBO(m_resourceContext, vertexElements, bufferSize);
	}
	else
		return new VertexBufferVAR(m_resourceContext, vertexElements, bufferSize, dynamic);

#else
	return new VertexBufferVAR(m_resourceContext, vertexElements, bufferSize, dynamic);
#endif
}

Ref< IndexBuffer > RenderSystemOpenGL::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

#if defined(_WIN32)

	if (glGenBuffersARB)
		return new IndexBufferIBO(m_resourceContext, indexType, bufferSize, dynamic);
	else
		return new IndexBufferIAR(m_resourceContext, indexType, bufferSize);

#elif defined(__APPLE__) || defined(__LINUX__)

	if (opengl_have_extension(E_GL_ARB_vertex_buffer_object))
		return new IndexBufferIBO(m_resourceContext, indexType, bufferSize, dynamic);
	else
		return new IndexBufferIAR(m_resourceContext, indexType, bufferSize);

#else
	return new IndexBufferIAR(m_resourceContext, indexType, bufferSize);
#endif
}

Ref< ISimpleTexture > RenderSystemOpenGL::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< SimpleTextureOpenGL > texture = new SimpleTextureOpenGL(m_resourceContext);
	if (!texture->create(desc, m_maxAnisotrophy))
		return 0;

	return texture;
}

Ref< ICubeTexture > RenderSystemOpenGL::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< CubeTextureOpenGL > texture = new CubeTextureOpenGL(m_resourceContext);
	if (!texture->create(desc))
		return 0;

	return texture;
}

Ref< IVolumeTexture > RenderSystemOpenGL::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< VolumeTextureOpenGL > texture = new VolumeTextureOpenGL(m_resourceContext);
	if (!texture->create(desc))
		return 0;

	return texture;
}

Ref< RenderTargetSet > RenderSystemOpenGL::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< RenderTargetSetOpenGL > renderTargetSet = new RenderTargetSetOpenGL(m_resourceContext, m_blitHelper);
	if (!renderTargetSet->create(desc))
		return 0;

	return renderTargetSet;
}

Ref< IProgram > RenderSystemOpenGL::createProgram(const ProgramResource* programResource)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);
	return ProgramOpenGL::create(m_resourceContext, programResource);
}

Ref< IProgramCompiler > RenderSystemOpenGL::createProgramCompiler() const
{
	return new ProgramCompilerOpenGL();
}

void RenderSystemOpenGL::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
