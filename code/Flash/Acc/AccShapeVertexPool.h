#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class VertexBuffer;

	}

	namespace flash
	{

class AccShapeVertexPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Range
	{
		render::VertexBuffer* vertexBuffer;

		Range()
		:	vertexBuffer(0)
		{
		}
	};

	AccShapeVertexPool(render::IRenderSystem* renderSystem, uint32_t frames, const AlignedVector< render::VertexElement >& vertexElements);

	bool create();

	void destroy();

	bool acquireRange(int32_t vertexCount, Range& outRange);

	void releaseRange(const Range& range);

	void cycleGarbage();

private:
	struct VertexRange
	{
		Ref< render::VertexBuffer > vertexBuffer;
		int32_t vertexCount;
	};

	typedef std::list< VertexRange > vr_list_t;

	Ref< render::IRenderSystem > m_renderSystem;
	AlignedVector< render::VertexElement > m_vertexElements;
	vr_list_t m_usedRanges;
	vr_list_t m_freeRanges;
	AlignedVector< vr_list_t > m_garbageRanges;
	uint32_t m_frame;
};

	}
}

