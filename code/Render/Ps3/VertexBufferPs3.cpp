#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/StateCachePs3.h"
#include "Render/Ps3/VertexBufferPs3.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t cg_attr_index(DataUsage usage, int index)
{
	const int32_t base[] =
	{
		0,	// DuPosition
		1,	// DuNormal
		2,	// DuTangent
		3,	// DuBinormal
		4,	// DuColor
		5,	// DuCustom
		16	// Last attribute index
	};
	int32_t attr = base[int(usage)] + index;
	T_ASSERT (attr < base[int(usage) + 1]);
	return attr;
}

		}

VertexBufferPs3* VertexBufferPs3::ms_activeVertexBuffer = 0;

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferPs3", VertexBufferPs3, VertexBuffer)

VertexBufferPs3::VertexBufferPs3(const AlignedVector< VertexElement >& vertexElements, MemoryHeapObject* vbo, int bufferSize, int32_t& counter)
:	VertexBuffer(bufferSize)
,	m_vbo(vbo)
,	m_counter(counter)
{
	m_vertexStride = getVertexSize(vertexElements);

	std::memset(m_attributeDesc, 0, sizeof(m_attributeDesc));

	for (AlignedVector< VertexElement >::const_iterator i = vertexElements.begin(); i != vertexElements.end(); ++i)
	{
		int32_t attr = cg_attr_index(i->getDataUsage(), i->getIndex());
		switch (i->getDataType())
		{
		case DtFloat1:
			m_attributeDesc[attr].size = 1;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtFloat2:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtFloat3:
			m_attributeDesc[attr].size = 3;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtFloat4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtByte4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_UB256;
			break;

		case DtByte4N:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_UB;
			break;

		case DtShort2:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S32K;
			break;

		case DtShort4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S32K;
			break;

		case DtShort2N:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S1;

		case DtShort4N:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S1;
			break;

		case DtHalf2:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_SF;
			break;

		case DtHalf4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_SF;
			break;
		}

		m_attributeDesc[attr].offset = i->getOffset();
	}

	++m_counter;
}

VertexBufferPs3::~VertexBufferPs3()
{
	destroy();
}

void VertexBufferPs3::destroy()
{
	if (ms_activeVertexBuffer == this)
	{
		for (int i = 0; i < sizeof_array(m_attributeDesc); ++i)
		{
			cellGcmSetVertexDataArray(
				gCellGcmCurrentContext,
				i,
				0,
				0,
				0,
				CELL_GCM_VERTEX_F,
				CELL_GCM_LOCATION_LOCAL,
				0
			);
		}

		ms_activeVertexBuffer = 0;
	}

	if (m_vbo)
	{
		m_vbo->free(); m_vbo = 0;
		--m_counter;
	}
}

void* VertexBufferPs3::lock()
{
	return m_vbo->getPointer();
}

void VertexBufferPs3::unlock()
{
}

void VertexBufferPs3::bind(StateCachePs3& stateCache, const AlignedVector< uint8_t >& signature)
{
	for (int i = 0; i < sizeof_array(m_attributeDesc); ++i)
	{
		if (m_attributeDesc[i].size && signature[i])
		{
			stateCache.setVertexDataArray(
				i,
				m_vertexStride,
				m_attributeDesc[i].size,
				m_attributeDesc[i].type,
				m_vbo->getLocation(),
				m_vbo->getOffset() + m_attributeDesc[i].offset
			);
		}
		else
		{
			stateCache.setVertexDataArray(
				i,
				0,
				0,
				CELL_GCM_VERTEX_F,
				CELL_GCM_LOCATION_LOCAL,
				0
			);
		}
	}
	ms_activeVertexBuffer = this;
}

	}
}