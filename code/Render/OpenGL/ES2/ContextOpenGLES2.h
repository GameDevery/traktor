#ifndef traktor_render_ContextOpenGLES2_H
#define traktor_render_ContextOpenGLES2_H

#include <map>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/TypesOpenGL.h"
#include "Render/OpenGL/IContext.h"

namespace traktor
{
	namespace render
	{
	
class EAGLContextWrapper;

/*! \brief OpenGL ES2 context.
 * \ingroup OGL
 */
class ContextOpenGLES2 : public IContext
{
	T_RTTI_CLASS;

public:
	static Ref< ContextOpenGLES2 > createResourceContext();

	static Ref< ContextOpenGLES2 > createContext(
		ContextOpenGLES2* resourceContext,
		void* nativeWindowHandle,
		uint32_t depthBits,
		uint32_t stencilBits
	);

	virtual bool enter();

	virtual void leave();

	virtual void deleteResource(IDeleteCallback* callback);

	virtual void deleteResources();

	GLuint createShaderObject(const char* shader, GLenum shaderType);

	bool resize(int32_t width, int32_t height);

	int32_t getWidth() const;

	int32_t getHeight() const;

	bool getLandscape() const;

	void swapBuffers();

	Semaphore& lock();

	void setRenderState(const RenderState& renderState);
	
	void bindPrimary();

private:
	static ThreadLocal ms_contextStack;
#if defined(TARGET_OS_IPHONE)
	EAGLContextWrapper* m_context;
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	EGLDisplay m_display;
	EGLSurface m_surface;
	EGLContext m_context;
#endif
	Semaphore m_lock;
	std::vector< IDeleteCallback* > m_deleteResources;
	std::map< uint32_t, GLuint > m_shaderObjects;
	RenderState m_renderState;

#if defined(TARGET_OS_IPHONE)
	ContextOpenGLES2(EAGLContextWrapper* context);
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	ContextOpenGLES2(EGLDisplay display, EGLSurface surface, EGLConfig context);
#endif
};

	}
}

#endif	// traktor_render_ContextOpenGLES2_H
