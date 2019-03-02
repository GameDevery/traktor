#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Flash/Shape.h"
#include "Flash/ShapeInstance.h"
#include "Flash/Debug/ShapeInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ShapeInstanceDebugInfo", 0, ShapeInstanceDebugInfo, InstanceDebugInfo)

ShapeInstanceDebugInfo::ShapeInstanceDebugInfo()
:	m_mask(false)
,	m_clipped(false)
{
}

ShapeInstanceDebugInfo::ShapeInstanceDebugInfo(const ShapeInstance* instance, bool mask, bool clipped)
{
	m_name = instance->getName();
	m_bounds = instance->getBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_shape = instance->getShape();
	m_mask = mask;
	m_clipped = clipped;
}

void ShapeInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> MemberRef< const Shape >(L"shape", m_shape);
	s >> Member< bool >(L"mask", m_mask);
	s >> Member< bool >(L"clipped", m_clipped);
}

	}
}
