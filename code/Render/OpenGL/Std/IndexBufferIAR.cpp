#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/IndexBufferIAR.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferIAR", IndexBufferIAR, IndexBufferOpenGL)

IndexBufferIAR::IndexBufferIAR(ContextOpenGL* context, IndexType indexType, uint32_t bufferSize)
:	IndexBufferOpenGL(indexType, bufferSize)
,	m_context(context)
,	m_data(0)
{
	m_data = new GLubyte [bufferSize + sizeof(GLuint)];
	T_ASSERT (m_data);

	memset(m_data, 0, bufferSize);
	*(GLuint*)&m_data[bufferSize] = 0xe1e1e1e1;
}

IndexBufferIAR::~IndexBufferIAR()
{
	destroy();
}

void IndexBufferIAR::destroy()
{
	delete[] m_data;
	m_data = 0;
}

void* IndexBufferIAR::lock()
{
	return m_data;
}

void IndexBufferIAR::unlock()
{
	T_ASSERT_M (*(GLuint*)&m_data[getBufferSize()] == 0xe1e1e1e1, L"Buffer overrun");
}

void IndexBufferIAR::bind()
{
}

const GLvoid* IndexBufferIAR::getIndexData() const
{
	T_ASSERT (m_data);
	return (const GLvoid*)m_data;
}

	}
}
