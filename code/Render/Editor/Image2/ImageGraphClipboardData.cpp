#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Image2/ImageGraphClipboardData.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphClipboardData", 0, ImageGraphClipboardData, ISerializable)

void ImageGraphClipboardData::addNode(Node* node)
{
	m_nodes.push_back(node);
}

void ImageGraphClipboardData::addEdge(Edge* edge)
{
	m_edges.push_back(edge);
}

void ImageGraphClipboardData::setBounds(const ui::Rect& bounds)
{
	m_bounds = bounds;
}

const RefArray< Node >& ImageGraphClipboardData::getNodes() const
{
	return m_nodes;
}

const RefArray< Edge >& ImageGraphClipboardData::getEdges() const
{
	return m_edges;
}

const ui::Rect& ImageGraphClipboardData::getBounds() const
{
	return m_bounds;
}

void ImageGraphClipboardData::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);
	s >> Member< int32_t >(L"boundsLeft", m_bounds.left);
	s >> Member< int32_t >(L"boundsTop", m_bounds.top);
	s >> Member< int32_t >(L"boundsRight", m_bounds.right);
	s >> Member< int32_t >(L"boundsBottom", m_bounds.bottom);
}

	}
}
