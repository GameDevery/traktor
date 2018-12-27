/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferStaticOpenGLES2_H
#define traktor_render_VertexBufferStaticOpenGLES2_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;
class StateCache;
class VertexElement;

/*!
 * \ingroup OGL
 */
class VertexBufferStaticOpenGLES2 : public VertexBufferOpenGLES2
{
	T_RTTI_CLASS;

public:
	VertexBufferStaticOpenGLES2(ContextOpenGLES2* context, const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual ~VertexBufferStaticOpenGLES2();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	virtual void activate(StateCache* stateCache) override final;

private:
	struct AttributeDesc
	{
		GLint location;
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLuint offset;
	};

	Ref< ContextOpenGLES2 > m_context;
	AlignedVector< AttributeDesc > m_attributes;
	GLuint m_arrayObject;
	GLuint m_bufferObject;
	GLuint m_vertexStride;
	AutoPtr< uint8_t, AllocFreeAlign > m_buffer;
	GLintptr m_lockOffset;
	GLsizeiptr m_lockSize;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_VertexBufferStaticOpenGLES2_H
