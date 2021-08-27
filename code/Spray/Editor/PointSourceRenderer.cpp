#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Sources/PointSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSourceRenderer", PointSourceRenderer, SourceRenderer)

void PointSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const PointSourceData* pointSource = checked_type_cast< const PointSourceData* >(sourceData);

	Vector4 position = pointSource->m_position;

	const Vector4 c_size(0.1f, 0.1f, 0.1f, 0.0f);
	primitiveRenderer->drawWireAabb(Aabb3(position - c_size, position + c_size), 1.0f, Color4ub(255, 255, 0));
}

	}
}
