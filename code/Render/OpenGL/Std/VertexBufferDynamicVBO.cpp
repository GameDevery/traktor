#include <cstring>
#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/VertexBufferDynamicVBO.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public IContext::IDeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffersARB(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicVBO", VertexBufferDynamicVBO, VertexBufferOpenGL)

VertexBufferDynamicVBO::VertexBufferDynamicVBO(IContext* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize)
:	VertexBufferOpenGL(bufferSize)
,	m_resourceContext(resourceContext)
,	m_lock(0)
,	m_dirty(false)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT (m_vertexStride > 0);

	T_OGL_SAFE(glGenBuffersARB(1, &m_name));
	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));
	T_OGL_SAFE(glBufferDataARB(GL_ARRAY_BUFFER_ARB, bufferSize, 0, GL_DYNAMIC_DRAW_ARB));

	std::memset(m_attributeDesc, 0, sizeof(m_attributeDesc));

	for (size_t i = 0; i < vertexElements.size(); ++i)
	{
		if (vertexElements[i].getIndex() >= 4)
		{
			log::warning << L"Index out of bounds on vertex element " << uint32_t(i) << Endl;
			continue;
		}

		int usageIndex = T_OGL_USAGE_INDEX(vertexElements[i].getDataUsage(), vertexElements[i].getIndex());
		switch (vertexElements[i].getDataType())
		{
		case DtFloat1:
			m_attributeDesc[usageIndex].size = 1;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtFloat2:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtFloat3:
			m_attributeDesc[usageIndex].size = 3;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtFloat4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtByte4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_UNSIGNED_BYTE;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtByte4N:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_UNSIGNED_BYTE;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtShort2:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtShort4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtShort2N:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtShort4N:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtHalf2:
			if (opengl_have_extension(E_GL_ARB_half_float_vertex))
			{
				m_attributeDesc[usageIndex].size = 2;
				m_attributeDesc[usageIndex].type = GL_HALF_FLOAT_ARB;
				m_attributeDesc[usageIndex].normalized = GL_TRUE;
			}
			else
				log::error << L"Unsupported vertex format; OpenGL driver doesn't support GL_ARB_half_float_vertex" << Endl;
			break;

		case DtHalf4:
			if (opengl_have_extension(E_GL_ARB_half_float_vertex))
			{
				m_attributeDesc[usageIndex].size = 4;
				m_attributeDesc[usageIndex].type = GL_HALF_FLOAT_ARB;
				m_attributeDesc[usageIndex].normalized = GL_TRUE;
			}
			else
				log::error << L"Unsupported vertex format; OpenGL driver doesn't support GL_ARB_half_float_vertex" << Endl;
			break;

		default:
			log::error << L"Unsupport vertex format" << Endl;
		}

		m_attributeDesc[usageIndex].offset = vertexElements[i].getOffset();
	}
	
	m_buffer.resize(getBufferSize(), 0);
	m_dirty = true;
}

VertexBufferDynamicVBO::~VertexBufferDynamicVBO()
{
	destroy();
}

void VertexBufferDynamicVBO::destroy()
{
	if (m_name)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
	
	m_buffer.resize(0);
}

void* VertexBufferDynamicVBO::lock()
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");
	
	m_lock = &m_buffer[0];
	return m_lock;
}

void* VertexBufferDynamicVBO::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");

	m_lock = &m_buffer[0];
	if (!m_lock)
		return 0;

	return (uint8_t*)m_lock + vertexOffset * m_vertexStride;
}

void VertexBufferDynamicVBO::unlock()
{
	T_ASSERT_M(m_lock, L"Vertex buffer not locked");

	m_lock = 0;
	m_dirty = true;
	
	setContentValid(true);
}

void VertexBufferDynamicVBO::activate(const GLint* attributeLocs)
{
	T_ASSERT_M(!m_lock, L"Vertex buffer still locked");

	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));
	
	if (m_dirty)
	{
		GLvoid* mapped = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		if (!mapped)
			return;
			
		std::memcpy(mapped, &m_buffer[0], getBufferSize());
		
		T_OGL_SAFE(glUnmapBufferARB(GL_ARRAY_BUFFER_ARB));
		m_dirty = false;
	}

	for (int i = 0; i < T_OGL_MAX_USAGE_INDEX; ++i)
	{
		if (attributeLocs[i] == -1 || m_attributeDesc[i].size == 0)
			continue;

		T_OGL_SAFE(glEnableVertexAttribArrayARB(attributeLocs[i]));
		T_OGL_SAFE(glVertexAttribPointerARB(
			attributeLocs[i],
			m_attributeDesc[i].size,
			m_attributeDesc[i].type,
			m_attributeDesc[i].normalized,
			m_vertexStride,
			(GLvoid*)m_attributeDesc[i].offset
		));
	}
}

	}
}
